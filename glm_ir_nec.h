#include "esphome.h"

#define SEND_NEC 1

using namespace esphome;

//#define GLM_DEBUG_NEC

#ifdef GLM_DEBUG_NEC
static const char *TAGnec = "ir.nec";
#endif

#include "IRsend.h"

#ifndef IR_INCLUDE_CPP
#define IR_INCLUDE_CPP
#include "irsend.cpp"
#include "irtimer.cpp"
#include "irrecv.cpp"
#include "irutils.cpp"
#endif
#include "ir_NEC.cpp"

class NecComponent;
class NecCommand : public switch_::Switch
{
    public:
        NecCommand(NecComponent* parent, uint16_t address, uint16_t command)
        {
            this->address_ = address;
            this->command_ = command;
        }

        void write_state(bool state) override;

        uint16_t get_address() { return address_; }
        uint16_t get_command() { return command_; }

    protected:
        NecComponent* parent_;
        uint16_t address_;
        uint16_t command_;
};

class NecComponent : public Component
{
  public:
    explicit NecComponent(uint8_t pin)
    {
        this->pin_ = pin;
    }

    void sendCommand(NecCommand* command)
    {
        IRsend ir_sender(this->pin_);

        #ifdef GLM_DEBUG_NEC
        ESP_LOGI(TAGnec, "Sending address %d command %d", command->get_address(), command->get_command());
        #endif

        //uint32_t cmd = ir_sender.encodeNEC(command->get_address(), command->get_command());
        //ESP_LOGI("main", "Sending address %d command %d -- ", command->get_address(), command->get_command());

        ir_sender.sendNEC(0xFD6897UL, kNECBits, kNoRepeat);
    }

    switch_::Switch *get_cmd(uint16_t address, uint16_t command)
    {
        return new NecCommand(this, address, command);
    }
  protected:
    uint8_t pin_;
};

void NecCommand::write_state(bool state)
{
    #ifdef GLM_DEBUG_NEC
    ESP_LOGI(TAGnec, "Switch set: %d", state);
    #endif
    parent_->sendCommand(this);

}