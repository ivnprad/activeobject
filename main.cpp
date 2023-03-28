#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>




class ActiveObject {
public:
    ActiveObject() : m_thread([this]() { run(); }){ // we pass lambda function because we want to capture this and assure that run has access to membervariables

    }
    ~ActiveObject() {
        {
            //std::unique_lock<std::mutex> lock(m_mutex);
            //m_stopped = true;
            m_stopped.store(true);
            m_condition.notify_all();
        }
        m_thread.join();
    }
    void send(std::function<void()> message) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_messageQueue.push(message);
        }
        m_condition.notify_all();
    }
private:
    void run() {
        while (!m_stopped.load()) {
            std::function<void()> message;
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                //while (m_messageQueue.empty() && !m_stopped) {
                   // m_condition.wait(lock);
                //}
                m_condition.wait(lock,[&]{return !(m_messageQueue.empty()) || m_stopped.load();}); // this replaces (while condition)

                if (!m_messageQueue.empty()) {
                    message = m_messageQueue.front();
                    m_messageQueue.pop();
                }
            }
            if (message) {
                message();
            }
        }
    }

    std::thread m_thread;
    std::queue<std::function<void()>> m_messageQueue;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    //bool m_stopped = false;
    std::atomic_bool m_stopped=false;
};






int main()
{

    ActiveObject myObject;

    myObject.send([]() {
        std::cout<<" first event "<<std::endl;
    });

    myObject.send([]() {
        std::cout<<" second event"<<std::endl;
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout<<" hello world "<< std::endl;
    return 0;  

}