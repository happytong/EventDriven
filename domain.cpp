// domain.cpp



#include "event_system.h"

//-----------------------------------
// Domain-specific Event Types
//-----------------------------------
struct TemperatureEvent {
    double value;
};

struct DoorStatusEvent {
    bool isOpen;
    int sensorId;
};

//-----------------------------------
// Domain Event Instances
//-----------------------------------
Event<TemperatureEvent> temperatureEvent;
Event<DoorStatusEvent> doorStatusEvent;

//-----------------------------------
// Domain Classes
//-----------------------------------
class TemperatureSensor {
public:
    void update(double temp) {
        temperatureEvent.trigger(TemperatureEvent{temp});
    }
};

class ClimateController {
public:
    ClimateController(EventsManager& em) {
        em.subscribe(temperatureEvent, [this](const TemperatureEvent& e) {
            std::cout << "Climate control received temperature: " 
                      << e.value << "°C\n";
            adjustSystem(e.value);
        }, "temperatureEvent");
    }

private:
    void adjustSystem(double temp) {
        if(temp > 25.0) std::cout << "Activating cooling system\n";
        else if(temp < 18.0) std::cout << "Activating heating system\n";
    }
};

class DoorMonitor {
public:
    void checkDoor(int sensorId, bool isOpen) {
        doorStatusEvent.trigger(DoorStatusEvent{isOpen, sensorId});
    }
};

class SecuritySystem {
public:
    SecuritySystem(EventsManager& em) {
        em.subscribe(doorStatusEvent, [](const DoorStatusEvent& e) {
            if(e.isOpen) {
                std::cout << "ALERT: Door " << e.sensorId 
                          << " opened unexpectedly!\n";
            }
			else std::cout << "Door status ok " << e.sensorId <<std::endl;
				
        }, "doorStatusEvent");
    }
};

//-----------------------------------
// Usage Example
//-----------------------------------
int main() {
    EventsManager em;
    TemperatureSensor sensor;
    ClimateController climate(em);
    DoorMonitor doorMonitor;
    SecuritySystem security(em);

    // Simulate system updates
    sensor.update(22.5);      // Normal temperature
    sensor.update(27.3);      // High temperature
    doorMonitor.checkDoor(1, false);  // Normal door status
    doorMonitor.checkDoor(2, true);   // Security alert
	std::cout << "done" << std::endl;

    return 0;
}