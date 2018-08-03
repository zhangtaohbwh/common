/*
 * 消息格式： head + body,其中head的长度固定，为8字节
 */
#ifndef MESSAGE_H
#define MESSAGE_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <assert.h>

 // 使分配的内存空间是8字节的整数倍（+1表示剩余不足8字节的要分配8字节的空间）
#define ALLOCTOR_SIZE(n) ((n / 8 + 1) * 8 + header_length)

class message
{
public:
	enum { header_length = 8 };
	enum { max_body_length = 512 };

    message()
        :data_(NULL),
        buf_length_(0),
		body_length_(0),
        msg_id_(0)
    {
	}

	message(const message &msg)
	{
		if (msg.body_length() != 0)
		{
			buf_length_ = ALLOCTOR_SIZE(msg.body_length());
			data_ = new char[buf_length_];
			memset(data_, 0, buf_length_);

			body_length_ = msg.body_length();
			msg_id_ = msg.get_msg_id();

			std::memcpy(data_, msg.data(), msg.length());
			fill_end(body_length_);
		}
	}

	~message()
	{
		free();
	}

	message& operator=(const message &rhs)
	{
		if (this != &rhs)
		{
			//当前缓存小于要存储数据长度
			if (header_length + body_length_ < rhs.length())
			{
				buf_length_ = ALLOCTOR_SIZE(rhs.body_length());
                delete [] data_;
				data_ = new char[buf_length_];
				memset(data_, 0, buf_length_);
			}

			msg_id_      = rhs.get_msg_id();
			body_length_ = rhs.body_length();
			std::memcpy(data_, rhs.data(), rhs.length());
			fill_end(body_length_);
		}

		return *this;
    }

    const char* data() const
    {
        return data_;
    }

	char* data(size_t pos = 0)
	{
		return data_ + pos;
	}

    size_t buf_length()
    {
        return buf_length_;
    }

	size_t length() const
	{
		return header_length + body_length_;
	}

    const char* body() const
    {
        return data_ + header_length;
    }

	char* body()
	{
		return data_ + header_length;
	}

	size_t body_length() const
	{
		return body_length_;
	}

    void set_body_length(size_t len)
    {
        body_length_ = len;
    }

	/*
	* @brief 获取缓冲区首部8个字节的内容，即消息的长度
	*
	*/
	bool decode_header()
	{
		char header[header_length + 1] = "";
		std::strncat(header, data_, header_length);
		body_length_ = std::atoi(header);
		if (body_length_ > max_body_length)
		{
			body_length_ = 0;
			return false;
		}
		return true;
	}
 
	/*
	* @brief 将消息的长度存储在缓冲区首部8个字节的位置
	*
	*/
	void encode_header()
	{
		if (0 == data_)
			return;

		char header[header_length + 1] = "";
		sprintf(header, "%8d", static_cast<int>(body_length_));
		std::memcpy(data_, header, header_length);
	}

	/*
	* @brief 填充要发送的消息
	*
	* @param szMsg 消息内容
	*
	* @note false-填充失败  true-填充成功
	*/
    bool fill_msg(const char *szMsg)
	{
		size_t len = strlen(szMsg);

		// 如果要发送的消息长度为0，那么就不能填充消息内容
		if (len == 0)
			return false;

		// 如果要发送的消息长度比缓冲区的长度还多header_length+1，那么需要重新分配内存
        if (len > (buf_length_ + header_length+1))
		{
			if (data_)
			{
				delete[] data_;
			}

			buf_length_ = ALLOCTOR_SIZE(len);

            data_ = new char[buf_length_];
            memset(data_, 0, buf_length_);
		}
		else if (NULL == data_)
		{
			buf_length_ = ALLOCTOR_SIZE(len);
			data_ = new char[buf_length_];
			memset(data_, 0, buf_length_);
		}

        body_length_ = len;

		encode_header();
		std::memcpy(data_ + header_length, szMsg, body_length_);
		fill_end(body_length_);

		return true;
	}

	/*
	* @brief 在指定位置[pos]插入字符串结束符
	*
	*/
	void fill_end(int pos)
	{
		assert(data_);

		// 如果要插入的位置不是有效的位置，则返回
		if ( (pos < 0) || (pos > (header_length + body_length_)) )
			return;

		data_[header_length+pos] = 0;
	}

	/*
	 * @brief 重新分配缓存大小
	 * 
	 * @param len 消息的有效长度
	 *
	 * @note 重新分配的缓存大小为： 8字节的整数倍
	 */
    bool realloc(size_t len)
	{
        if (len <= 0)
            return false;

        if (data_)
            delete[] data_;// 删除旧的缓存

        buf_length_  = ALLOCTOR_SIZE(len);
        data_        = new char[buf_length_];

		return true;
	}

	/*
	* @brief 释放缓存
	*
	*/
	void free()
	{
		assert(data_);
		delete[] data_;
		data_ = NULL;

        buf_length_ = 0;
        body_length_ = 0;
	}

	void set_msg_id(size_t msg_id)
	{
		msg_id_ = msg_id;
	}

	size_t get_msg_id() const
	{
		return msg_id_;
	}

private:
    char   *data_;          // 存储消息的缓存（消息格式为：8字节的head + body）
    size_t buf_length_;     // data_的大小（大小为8字节的整数倍）
    size_t body_length_;    // 不包括字符串的结束符'\0'
    size_t msg_id_;			// 消息id
};



#endif // MESSAGE_H
