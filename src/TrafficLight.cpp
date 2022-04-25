#include <iostream>
#include <chrono>
#include <random>
#include "TrafficLight.h"


/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    std::unique_lock<std::mutex> lck2(_mtx1);
    _conditionVariable.wait(lck2, [this] { return !_queue.empty(); } );

    // remove
    T queue = std::move(_queue.back());
    _queue.pop_back();

    return queue;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck1(_mtx1);
    std::cout << "Message #" << msg << " will be added to queue.\n";
    _queue.clear();
    _queue.push_back(std::move(msg));
    _conditionVariable.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight(){
    for( auto &t : threads ){
        t.join();
    }
}
void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true){
        _messageQueue.receive();
        if(TrafficLightPhase::green){
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. 
    // To do this, use the thread queue in the base class. 
    std::thread t = std::thread(&TrafficLight::cycleThroughPhases, this);
    threads.emplace_back(std::move(t));

}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    std::random_device rdm;
    std::mt19937 range(rdm());
    std::uniform_int_distribution<std::mt19937::result_type> result(4000,6000);
    long randomNumber = result(range);

    auto timeStart = std::chrono::high_resolution_clock::now();

    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        auto timeStop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(timeStop - timeStart).count();
        
        if(duration >= randomNumber){
            if(_currentPhase == TrafficLightPhase::green){
                _currentPhase = TrafficLightPhase::red;
                timeStart = std::chrono::high_resolution_clock::now();
            }else{
                _currentPhase = TrafficLightPhase::green;
                timeStart = std::chrono::high_resolution_clock::now();
            }
            randomNumber = result(range);
            _messageQueue.send(std::move(_currentPhase));
        }

    }

}