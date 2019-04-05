#include "esphome.h"

#define SEND_COOLIX 1

using namespace esphome;

static const char *TAGcoolix = "ac.coolix";


#include "IRsend.h"
#include "ir_Coolix.h"

#include "irsend.cpp"
#include "ir_Coolix.cpp"
#include "irtimer.cpp"
#include "irrecv.cpp"
#include "irutils.cpp"

class CoolixComponent;
class CoolixCommand : public switch_::Switch
{
    public:
        CoolixCommand(CoolixComponent* parent, bool power, int8_t mode, int8_t temp)
        {
            this->parent_ = parent;
            this->mode_ = mode;
            this->temp_ = temp;
        }

        void write_state(bool state) override;

        bool get_power() {return power_; }
        int8_t get_mode() { return mode_; }
        int8_t get_temp() { return temp_; }

    protected:
        CoolixComponent* parent_;
        bool power_;
        int8_t mode_;
        int8_t temp_;
};

class CoolixComponent : public Component
{
  public:
    explicit CoolixComponent(esphome::remote::RemoteTransmitterComponent* transmitter)
    {
        this->transmitter_ = transmitter;
    }

    void sendCommand(CoolixCommand* command)
    {
        IRCoolixAC coolix(14);

        //ESP_LOGI(TAG, "Cmd AC mode: %d temp %d", command->get_mode(), command->get_temp());
        if (command->get_power())
        {
            coolix.setMode(command->get_mode());
            coolix.setTemp(command->get_temp());
            coolix.setFan(kCoolixFanMax);
        }
        else
            coolix.setPower(false);

        coolix.send(1);
    }

    switch_::Switch *get_cmd(bool power, int8_t mode, int8_t temp)
    {
        return new CoolixCommand(this, power, mode, temp);
    }
  protected:
    esphome::remote::RemoteTransmitterComponent* transmitter_;
};

void CoolixCommand::write_state(bool state)
{
    ESP_LOGI(TAGcoolix, "Switch set: %d", state);
    parent_->sendCommand(this);

}