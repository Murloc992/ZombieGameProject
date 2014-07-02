#pragma once

class Timer
{
protected:
    uint32_t start_time;
    uint32_t time;
    uint32_t real_time;

    void update_real_time();

public:

    Timer();
    Timer(uint32_t begin_time);
    virtual ~Timer();

    uint32_t get_time();
    uint32_t get_start_time();
    void set_time(uint32_t time);

    uint32_t get_real_time();

    void tick();
};

typedef std::shared_ptr<Timer> timer_ptr;
