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
    std::unique_lock<std::mutex> ulock(_mx);
    _conVariable.wait(ulock,[this](){return !this->_queue.empty();});
    T mssg = _queue.front();
    _queue.pop_front();
    return mssg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
     // Lock mutex
    std::unique_lock uLock(_mx);

    // Add message to the queue
    _queue.emplace_back(std::move(msg));

    // Notify listening threads
    _conVariable.notify_one();
}


/* Implementation of class "TrafficLight" */

 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        if(_trafficLightQueue.receive() == TrafficLightPhase::green)
            return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread
    // when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    std::thread trafficLightSimulation(&TrafficLight::cycleThroughPhases,this);
    this->threads.emplace_back(std::move(trafficLightSimulation));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    auto start = std::chrono::system_clock::now();
    auto waitTime = ((rand()%3)+4)*1000; //wait  time from 4 to 6 sec
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        auto end = std::chrono::system_clock::now();
        auto deltaTime =std::chrono::duration_cast<std::chrono::milliseconds> (end-start).count();
        
        
        if(deltaTime >= waitTime)
        {
            waitTime = ((rand()%3)+4)*1000;  //wait  time from 4 to 6 sec
            start = end;
            this->_currentPhase = (_currentPhase ==TrafficLightPhase::red)? TrafficLightPhase::green : TrafficLightPhase::red;
            std::cout<<"DELTA "<<deltaTime<<"  waitTime "<<waitTime<<" "<<(_currentPhase == TrafficLightPhase::green)<<std::endl;
            //in messege queue pushback current state of traffic light
            _trafficLightQueue.send(std::move(_currentPhase));
        }

    }
}

