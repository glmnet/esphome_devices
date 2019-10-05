// Must disable logging if using logging in main.cpp or in other custom components for the
//  __c causes a section type conflict with __c thingy
// you can enable logging and use it if you enable this in logger:
/*
logger:
  level: DEBUG
  esp8266_store_log_strings_in_flash: False
  */

//#define APE_LOGGING

// take advantage of LOG_ defines to decide which code to include
#ifdef LOG_BINARY_OUTPUT
#define APE_BINARY_OUTPUT
#endif
#ifdef LOG_BINARY_SENSOR
#define APE_BINARY_SENSOR
#endif
#ifdef LOG_SENSOR
#define APE_SENSOR
#endif

static const char *TAGape = "ape";

#define APE_CMD_DIGITAL_READ 0
#define APE_CMD_WRITE_ANALOG 2
#define APE_CMD_WRITE_DIGITAL_HIGH 3
#define APE_CMD_WRITE_DIGITAL_LOW 4
#define APE_CMD_SETUP_PIN_OUTPUT 5
#define APE_CMD_SETUP_PIN_INPUT_PULLUP 6
#define APE_CMD_SETUP_PIN_INPUT 7
// 8 analog registers.. A0 to A7
// A4 and A5 not supported due to I2C
#define CMD_ANALOG_READ_A0 0b1000 // 0x8
// ....
#define CMD_ANALOG_READ_A7 0b1111 // 0xF

#define CMD_SETUP_ANALOG_INTERNAL 0x10
#define CMD_SETUP_ANALOG_DEFAULT 0x11

#define get_ape(constructor) static_cast<ArduinoPortExpander *> \
  (const_cast<custom_component::CustomComponentConstructor *>(&constructor)->get_component(0))

#define ape_binary_output(ape, pin) get_ape(ape)->get_binary_output(pin)
#define ape_binary_sensor(ape, pin) get_ape(ape)->get_binary_sensor(pin)
#define ape_analog_input(ape, pin) get_ape(ape)->get_analog_input(pin)

class ArduinoPortExpander;

using namespace esphome;

#ifdef APE_BINARY_OUTPUT
class ApeBinaryOutput : public output::BinaryOutput
{
public:
  ApeBinaryOutput(ArduinoPortExpander *parent, uint8_t pin)
  {
    this->parent_ = parent;
    this->pin_ = pin;
  }
  void write_state(bool state) override;
  uint8_t get_pin() { return this->pin_; }

protected:
  ArduinoPortExpander *parent_;
  uint8_t pin_;
  // Pins are setup as output after the state is written, Arduino has no open drain outputs, after setting an output it will either sink or source thus activating outputs writen to false during a flick.
  bool setup_{true};
  bool state_{false};

  friend class ArduinoPortExpander;
};
#endif

#ifdef APE_BINARY_SENSOR
class ApeBinarySensor : public binary_sensor::BinarySensor
{
public:
  ApeBinarySensor(ArduinoPortExpander *parent, uint8_t pin)
  {
    this->pin_ = pin;
  }
  uint8_t get_pin() { return this->pin_; }

protected:
  uint8_t pin_;
};
#endif

#ifdef APE_SENSOR
class ApeAnalogInput : public sensor::Sensor
{
public:
  ApeAnalogInput(ArduinoPortExpander *parent, uint8_t pin)
  {
    this->pin_ = pin;
  }
  uint8_t get_pin() { return this->pin_; }

protected:
  uint8_t pin_;
};
#endif

class ArduinoPortExpander : public Component, public I2CDevice
{
public:
  ArduinoPortExpander(I2CComponent *parent, uint8_t address, bool vref_default = false) : I2CDevice(parent, address)
  {
    this->vref_default_ = vref_default;
  }
  void setup() override
  {
#ifdef APE_LOGGING
    ESP_LOGCONFIG(TAGape, "Setting up ArduinoPortExpander at %#02x ...", address_);
#endif

    /* We cannot setup as usual as arduino boots later than esp8266

            Poll i2c bus for our Arduino for a n seconds instead of failing fast,
            also this is important as pin setup (INPUT_PULLUP, OUTPUT it's done once)
        */
    this->configure_timeout_ = millis() + 5000;
  }
  void loop() override
  {
    if (millis() < this->configure_timeout_)
    {
      bool try_configure = millis() % 100 > 50;
      if (try_configure == this->configure_)
        return;
      this->configure_ = try_configure;

      if (this->read_bytes(APE_CMD_DIGITAL_READ, const_cast<uint8_t *>(this->read_buffer_), 3, 1))
      {
#ifdef APE_LOGGING
        ESP_LOGCONFIG(TAGape, "ArduinoPortExpander found at %#02x", address_);
#endif
        delay(10);
        if (this->vref_default_)
        {
          this->write_byte(CMD_SETUP_ANALOG_DEFAULT, 0); // 0: unused
        }

        // Config success
        this->configure_timeout_ = 0;
        this->status_clear_error();
#ifdef APE_BINARY_SENSOR
        for (ApeBinarySensor *pin : this->input_pins_)
        {
          App.feed_wdt();
          uint8_t pinNo = pin->get_pin();
#ifdef APE_LOGGING
          ESP_LOGCONFIG(TAGape, "Setup input pin %d", pinNo);
#endif
          this->write_byte(APE_CMD_SETUP_PIN_INPUT_PULLUP, pinNo);
          delay(20);
        }
#endif
#ifdef APE_BINARY_OUTPUT
        for (ApeBinaryOutput *output : this->output_pins_)
        {
          if (!output->setup_)
          { // this output has a valid value already
            this->write_state(output->pin_, output->state_, true);
            App.feed_wdt();
            delay(20);
          }
        }
#endif
#ifdef APE_SENSOR
        for (ApeAnalogInput *sensor : this->analog_pins_)
        {
          App.feed_wdt();
          uint8_t pinNo = sensor->get_pin();
#ifdef APE_LOGGING
          ESP_LOGCONFIG(TAGape, "Setup analog input pin %d", pinNo);
#endif
          this->write_byte(APE_CMD_SETUP_PIN_INPUT, pinNo);
          delay(20);
        }
#endif
        return;
      }
      // Still not answering
      return;
    }
    if (this->configure_timeout_ != 0 && millis() > this->configure_timeout_)
    {
#ifdef APE_LOGGING
      ESP_LOGE(TAGape, "ArduinoPortExpander NOT found at %#02x", address_);
#endif
      this->mark_failed();
      return;
    }

#ifdef APE_BINARY_SENSOR
    if (!this->read_bytes(APE_CMD_DIGITAL_READ, const_cast<uint8_t *>(this->read_buffer_), 3, 1))
    {
#ifdef APE_LOGGING
      ESP_LOGE(TAGape, "Error reading. Reconfiguring pending.");
#endif
      this->status_set_error();
      this->configure_timeout_ = millis() + 5000;
      return;
    }
    for (ApeBinarySensor *pin : this->input_pins_)
    {
      uint8_t pinNo = pin->get_pin();

      uint8_t bit = pinNo % 8;
      uint8_t value = pinNo < 8 ? this->read_buffer_[0] : pinNo < 16 ? this->read_buffer_[1] : this->read_buffer_[2];
      bool ret = value & (1 << bit);
      if (this->initial_state_)
        pin->publish_initial_state(ret);
      else
        pin->publish_state(ret);
    }
#endif
#ifdef APE_SENSOR
    for (ApeAnalogInput *pin : this->analog_pins_)
    {
      uint8_t pinNo = pin->get_pin();
      pin->publish_state(analogRead(pinNo));
    }
#endif
    this->initial_state_ = false;
  }

#ifdef APE_SENSOR
  uint16_t analogRead(uint8_t pin)
  {
    bool ok = this->read_bytes((uint8_t)(CMD_ANALOG_READ_A0 + pin), const_cast<uint8_t *>(this->read_buffer_), 2, 1);
#ifdef APE_LOGGING
    ESP_LOGVV(TAGape, "analog read pin: %d ok: %d byte0: %d byte1: %d", pin, ok, this->read_buffer_[0], this->read_buffer_[1]);
#endif
    uint16_t value = this->read_buffer_[0] | ((uint16_t)this->read_buffer_[1] << 8);
    return value;
  }
#endif

#ifdef APE_BINARY_OUTPUT
  output::BinaryOutput *get_binary_output(uint8_t pin)
  {
    ApeBinaryOutput *output = new ApeBinaryOutput(this, pin);
    output_pins_.push_back(output);
    return output;
  }
#endif
#ifdef APE_BINARY_SENSOR
  binary_sensor::BinarySensor *get_binary_sensor(uint8_t pin)
  {
    ApeBinarySensor *binarySensor = new ApeBinarySensor(this, pin);
    input_pins_.push_back(binarySensor);
    return binarySensor;
  }
#endif
#ifdef APE_SENSOR
  sensor::Sensor *get_analog_input(uint8_t pin)
  {
    ApeAnalogInput *input = new ApeAnalogInput(this, pin);
    analog_pins_.push_back(input);
    return input;
  }
#endif
  void write_state(uint8_t pin, bool state, bool setup = false)
  {
    if (this->configure_timeout_ != 0)
      return;
#ifdef APE_LOGGING
    ESP_LOGD(TAGape, "Writing %d to pin %d", state, pin);
#endif
    this->write_byte(state ? APE_CMD_WRITE_DIGITAL_HIGH : APE_CMD_WRITE_DIGITAL_LOW, pin);
    if (setup)
    {
      App.feed_wdt();
      delay(20);
#ifdef APE_LOGGING
      ESP_LOGI(TAGape, "Setup output pin %d", pin);
#endif
      this->write_byte(APE_CMD_SETUP_PIN_OUTPUT, pin);
    }
  }

protected:
  bool configure_{true};
  bool initial_state_{true};
  uint8_t read_buffer_[3]{0, 0, 0};
  unsigned long configure_timeout_{5000};
  bool vref_default_{false};

#ifdef APE_BINARY_OUTPUT
  std::vector<ApeBinaryOutput *> output_pins_;
#endif
#ifdef APE_BINARY_SENSOR
  std::vector<ApeBinarySensor *> input_pins_;
#endif
#ifdef APE_SENSOR
  std::vector<ApeAnalogInput *> analog_pins_;
#endif
};

#ifdef APE_BINARY_OUTPUT
void ApeBinaryOutput::write_state(bool state)
{
  this->state_ = state;
  this->parent_->write_state(this->pin_, state, this->setup_);
  this->setup_ = false;
}
#endif