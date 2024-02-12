#include <memory>
#include <iostream>

#include <Poco/Timer.h>
#include <Poco/Thread.h>

class HeartbeatTimer
{
public:
    HeartbeatTimer(const std::string connectionID, const int timeout_ms) : _connectionID(connectionID), _timeout_ms(timeout_ms), _rxTimeElapsed(0), _txTimeElapsed(0), _alive(true)
    {
        _timer = std::make_unique<Poco::Timer>(250, 250); // TODO: Remove magic numbers
        _timer->start(Poco::TimerCallback<HeartbeatTimer>(*this, &HeartbeatTimer::OnTimer));
    }

    bool IsAlive()
    {
        return _alive;
    }

    void ResetHeartbeat()
    {
        std::cout << _connectionID << " - Resetting heartbeat" << '\n';
        _rxTimeElapsed = 0;
    }

    void OnTimer(Poco::Timer& timer)
    {
        auto timeInterval = timer.getPeriodicInterval();
        _rxTimeElapsed += timeInterval;
        _txTimeElapsed += timeInterval;

        if (_rxTimeElapsed > (_timeout_ms << 1))
        {
            std::cout << _connectionID << " Heartbeat timed out after " << _rxTimeElapsed << '\n';
            // OnHeartbeatTimeout()
            timer.restart(0); // Stop the timer
            _alive = false;
        }
        else if (_txTimeElapsed > _timeout_ms)
        {
            std::cout << _connectionID << " Send heartbeat after " << _txTimeElapsed << '\n';
            // SendHeartbeat()
            _txTimeElapsed = 0;
        }
    }

private:
    std::unique_ptr<Poco::Timer> _timer;
    const std::string _connectionID;
    const int _timeout_ms;
    int _rxTimeElapsed;
    int _txTimeElapsed;
    bool _alive;
};

int main(int argc, char const *argv[])
{
    (void)argc;
    (void)argv; // Ignore unused input variables

    std::cout << "Starting Heartbeat timers\n";
    auto handler = std::make_unique<HeartbeatTimer>("fgaa45-7556b2", 1100);
    auto handler2 = std::make_unique<HeartbeatTimer>("baa564-887da9", 2000);

    Poco::Thread::sleep(1000);
    handler->ResetHeartbeat();
    Poco::Thread::sleep(1000);
    handler->ResetHeartbeat();
    handler2->ResetHeartbeat();
    Poco::Thread::sleep(5000);

    return 0;
}