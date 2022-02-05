#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    // Perfore queue modification under the lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock,[this]{return !_messages.empty();}); // pass unique lock to condition variable

    //remove last vector element from queue
    T msg = std::move(_messages.back());
    _messages.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    //perfome vector modification under the lock
    std::lock_guard<std::mutex> ulock(_mutex);
    _messages.clear();
    _messages.emplace_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight(){
    
}

void TrafficLight::WaitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    TrafficLightPhase currentPhase = TrafficLightPhase::red;
    while (currentPhase != TrafficLightPhase::green ) {
        currentPhase = _trafficLightPhaseQueue.receive();
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::CycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::CycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    time_t current_time, total_time;
    while (true){
        std::random_device rd;
        std::mt19937 eng(rd());
        std::uniform_int_distribution<> distr(40,60);
        int cycle_duration = distr(eng);
        current_time = time(NULL);
        total_time = current_time + cycle_duration;
        while (current_time != total_time){
            current_time = time(NULL);
            // Sleep for 1 ms 
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        if(_currentPhase == TrafficLightPhase::red)
            _currentPhase = TrafficLightPhase::green;
        else
            _currentPhase = TrafficLightPhase::red;

        std::cout << "Sending the message" << std::endl;
        _trafficLightPhaseQueue.send(std::move(_currentPhase));
    }
    
}
