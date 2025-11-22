#include "Logger.hpp"
#include <string>
#include <ctime>
#include <iostream>
#include <iomanip>


Logger::Logger()
              :current_buf_(new std::string())
              ,next_buf_(new std::string())
              ,type_(INFO)
{
    file_.open("Log.txt",std::ios::out);

    if(!file_.is_open())
        std::cout<<"error"<<std::endl;
    current_buf_->reserve(BACKSIZE);
    next_buf_->reserve(BACKSIZE);

    thread_=std::move(std::thread([this](){ThreadFunc();}));
}
Logger *Logger::GetInstance()
{
    static Logger logger;
    return &logger;
}

void Logger::Log(LogType type, const char *buf)
{
    if(type<type_)
        return;
    std::string pre;
    switch (type)
    {
    case ERROR:
    {
        pre = "[ERROR]";
        break;
    }
    case INFO:
    {
        pre = "[INFO]";
        break;
    }
    case DEBUG:
    {
        pre = "[DEBUG]";
        break;
    }
    case WARNING:
    {
        pre= "[WARNING]";
        break;
    }
    }
    {
        
        auto now=std::chrono::system_clock::now();
        auto tt=std::chrono::system_clock::to_time_t(now);
        auto ltm=std::localtime(&tt);
        auto time_str=std::put_time(ltm,"%Y-%m-%d %H:%M:%S");

        std::stringstream ss;
        ss<<pre<<time_str<<" : "<<buf;
        //std::cout<<pre<<time_str<<" : "<<buf;
        auto tmp=ss.str();

        std::unique_lock<std::mutex> lock(mtx_);
        if(current_buf_->size()+tmp.size()>current_buf_->capacity())
        {
            buffers_.push_back(std::move(current_buf_));
            if(!next_buf_)
                current_buf_=std::move(next_buf_);
            else
            {
                current_buf_=BufferPtr(new std::string());
                current_buf_->reserve(BACKSIZE);
            }
            cv_.notify_one();
        }
        current_buf_->append(tmp);
    }

}

void Logger::ThreadFunc()
{
    BufferPtr back_buf1(new std::string()),back_buf2(new std::string());
    back_buf1->reserve(BACKSIZE);
    back_buf2->reserve(BACKSIZE);
    
    while(true)
    {
        std::vector<BufferPtr> buffers;
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait_for(lock,std::chrono::seconds(duration_));

            buffers_.push_back(std::move(current_buf_));
            if(next_buf_)
                current_buf_=std::move(next_buf_);
            else
                current_buf_=std::move(back_buf1);
            next_buf_=std::move(back_buf2);
            std::swap(buffers,buffers_);
        }

        for(auto& buf:buffers)
        {
            file_<<*buf;
            file_.flush();
            buf->clear();
            //std::cout<<*buf;
        }

        back_buf2=std::move(buffers.back());
        buffers.pop_back();
        if(!buffers.empty())
            back_buf1=std::move(buffers.back());
    }
}

LogType Logger::GetType() const
{
    return type_;
}

void Logger::SetType(LogType type)
{
    type_=type;
}