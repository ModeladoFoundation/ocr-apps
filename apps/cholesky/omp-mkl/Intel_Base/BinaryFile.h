/*
Intel Base Kernel Library: Binary File template class

Owned and maintained by the Extreme Scale Analytics Group (ESAG)
of the Innovation and Pathfinding Architecture Group (IPAG) of the Data Center Group (DCG)

Intel Corporation Proprietary, do not distribute externally without author permission (Brian.D.Womack@intel.com)
*/
#pragma once
#include "GU.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// Container for binary file that contains a header, then multiple records composed of a T based class, followed by a record count footer
// All member functions assume caller handles exceptions
template <class T> class BinaryFile
{
private:
    unsigned short _file_magic = 1122;
    unsigned int _record_size = 0;
    unsigned long long _record_count = 0;
    double _version = 0;
    string _file_path;
    fstream _file_stream;
    ios_base::openmode _file_mode;

public:
    typedef T RecordType;

    // Creates instance of binary FileStream, but does not open it.
    BinaryFile(const double &version = INTEL_BASE_KERNEL_VERSION, const string &filePath = "")
    {
        _version = version;
        _file_path = filePath;
        _record_size = sizeof(T);
    }

    // Destructs instance of binary FileStream, and closes it
    ~BinaryFile()
    {
        Close();
    }

    string FilePath() const { return _file_path; }
    string FilePath(const string &value) { return _file_path = value; }
    fstream &FileStream() { return _file_stream; }
    ios_base::openmode FileMode() const { return _file_mode; }
    ios_base::openmode FileMode(const ios_base::openmode &value)  { return _file_mode = value; }
    float FileVersion() const { return _version; }
    unsigned short FileMagic() const { return _file_magic; }
    unsigned int RecordSize() const { return _record_size; }
    unsigned long long RecordCount() const { return _record_count; }

    // All of these Open functions set _file_mode with mode, then open the file.  Returns true on success.
    bool OpenRead(const ios_base::openmode &mode = ios::binary | ios::in | ios::out)
    {
        Close();
        _file_mode = mode;
        if (Intel_Base::FileExists(_file_path))
        {
            _file_stream.open(_file_path, _file_mode);
            return _file_stream.is_open();
        }
        else
        {
            cerr << "BinaryFile::OpenRead File [" << _file_path << "] does not exist" << endl;
            return false;
        }
    }

    // Open file for reading and writing, usually used when the footer already written and want to edit records in middle)
    bool OpenReadWrite(const ios_base::openmode &mode = ios::binary | ios::in | ios::out)
    {
        Close();
        _file_mode = mode;
        if (Intel_Base::FileExists(_file_path))
        {
            _file_stream.open(_file_path, _file_mode);
            return _file_stream.is_open();
        }
        else
        {
            cerr << "BinaryFile::OpenReadWrite File [" << _file_path << "] does not exist" << endl;
            return false;
        }
    }

    // Open file for writing, getting rid of all content, so need to write header, records, and footer
    bool OpenWrite(const ios_base::openmode &mode = ios::binary | ios::out | ios::trunc)
    {
        Close();
        _record_count = 0;
        _file_mode = mode;
        _file_stream.open(_file_path, _file_mode);
        return _file_stream.is_open();
    }

    // Open file for writing, assuming the header has already been written, but not the footer
    bool OpenWriteAppend(const ios_base::openmode &mode = ios::binary | ios::out | ios::app)
    {
        Close();
        _record_count = 0;
        _file_mode = mode;
        _file_stream.open(_file_path, _file_mode);
        return _file_stream.is_open();
    }

    // Close FileStream if open
    void Close()
    {
        if (_file_stream.is_open()) _file_stream.close();
    }

    // Number of bytes required for file header
    unsigned int HeaderSize() { return sizeof(_version) + sizeof(_file_magic); }

    // OpenWrite, then add header with _version and _file_magic
    bool WriteHeader()
    {
        if (OpenWrite())
        {
            _file_stream.write((char*)&_version, sizeof(_version));
            _file_stream.write((char*)&_file_magic, sizeof(_file_magic));
            return true;
        }
        else return false;
    }

    // OpenRead, then get _version and _file_magic
    bool ReadHeader()
    {
        if (OpenRead())
        {
            _file_stream.read((char*)&_version, sizeof(_version));
            _file_stream.read((char*)&_file_magic, sizeof(_file_magic));
            return !_file_stream.eof();
        }
        else return false;
    }

    // After WriteHeader and all record writes, we append _record_count and close the file
    bool WriteFooter()
    {
        if (_file_stream.is_open() && (_file_mode & ios::out) > 0 && _record_count > 0)
        {
            _file_stream.write((char*)&_record_count, sizeof(_record_count));
            _file_stream.close();
            return true;
        }
        else
        {
            cerr << "BinaryFile::WriteFooter File " << (_file_stream.is_open() ? "is open" : "is not open")
                << " for " << ((_file_mode & ios::out) > 0 ? "writing" : "reading")
                << " and RecordCount is " << _record_count << endl;
            return false;
        }
    }

    // Position of starting byte for footer
    unsigned long long FooterStart()
    {
        unsigned long long file_size = Intel_Base::FileSize(_file_path), footer_start = file_size - sizeof(_record_count);
        //cout << "FooterStart: file_size " << file_size << "  footer_start " << footer_start << endl;
        return footer_start;
    }

    // Position _file_stream to end of file minus sizeof(_record_count) to return number of records, then return to previous position
    unsigned long long ReadFooter()
    {
        fstream::pos_type old_pos = 0;
        if (_file_stream.is_open() && (_file_mode & ios::in) > 0) old_pos = TellRead();
        else OpenRead();
        if (_file_stream.is_open())
        {
            SeekRead(FooterStart());
            _file_stream.read((char*)&_record_count, sizeof(_record_count));
            SeekRead(old_pos);
            return _record_count;
        }
        else return _record_count = 0;
    }

    // OpenRead, then position _file_stream at i'th record in file assuming it's less than _record_count and return true on success
    bool SeekReadRecord(unsigned long long i)
    {
        if ((_file_stream.is_open() && (_file_mode & ios::in) > 0) || OpenRead())
        {
            if (_record_size > 0)
            {
                if (i < _record_count)
                {
                    _file_stream.seekg(HeaderSize() + i * _record_size);
                    return true;
                }
                else cerr << "BinaryFile::SeekReadRecord could not access record " << i << " >= " << _record_count << " _record_count" << endl;
            }
            else cerr << "BinaryFile::SeekReadRecord File [" << _file_path << "] record_size is 0 length from constructor" << endl;
        }
        else cerr << "BinaryFile::SeekReadRecord File [" << _file_path << "] could not be opened" << endl;
        return false;
    }

    // OpenWrite, then position _file_stream at i'th record in file assuming it's less than or equal to _record_count and return true on success
    bool SeekWriteRecord(unsigned long long i)
    {
        if ((_file_stream.is_open() && (_file_mode & ios::out) > 0) || OpenWrite())
        {
            if (_record_size > 0)
            {
                if (i <= _record_count)
                {
                    _file_stream.seekg(HeaderSize() + i * _record_size);
                    return true;
                }
                else cerr << "BinaryFile::SeekWriteRecord could not access record " << i << " > " << _record_count << " _record_count" << endl;
            }
            else cerr << "BinaryFile::SeekWriteRecord File [" << _file_path << "] record_size is 0 length from constructor" << endl;
        }
        else cerr << "BinaryFile::SeekWriteRecord File [" << _file_path << "] could not be opened" << endl;
        return false;
    }

    // Set or Get file read or write position functions

    // _file_stream.seekg(position)
    void SeekRead(const fstream::pos_type &position)
    {
        _file_stream.seekg(position);
    }

    // _file_stream.seekp(position)
    void SeekWrite(const fstream::pos_type &position)
    {
        _file_stream.seekp(position);
    }

    // _file_stream.tellg()
    fstream::pos_type TellRead()
    {
        return _file_stream.tellg();
    }

    // _file_stream.tellp()
    fstream::pos_type TellWrite()
    {
        return _file_stream.tellp();
    }

    // Read a single T from FileStream at current position, else return false if at end of file
    bool Read(T &record)
    {
        if (_file_stream.eof()) return false;
        else
        {
            _file_stream.read((char*)&record, sizeof(record));
            return true;
        }
    }

    // Input stream to FileStream operator
    istream &operator >> (T &record)
    {
        _file_stream.read((char*)&record, sizeof(record));
        return _file_stream;
    }

    // If start_record == 0, and read all records if num_records == 0, else read num_records
    vector<T> ReadMany(const unsigned long long &start_record = 0, const unsigned long long &num_records = 0)
    {
        vector<T> records;
        if (ReadFooter() > 0 && OpenRead() && ReadHeader())
        {
            SeekReadRecord(start_record);
            unsigned long long get_records = num_records == 0 ? _record_count : num_records;
            if (start_record + get_records > _record_count) get_records = _record_count - start_record;
            records = vector<T>(get_records);
            for (unsigned long long i = 0; i < get_records; ++i)
            {
                if (!Read(records[i]))
                {
                    records.resize(i);
                    break;
                }
            }
        }
        else cerr << "ReadAll: File [" << _file_path << "] could not be opened" << endl;
        return records;
    }

    // Write a single record from FileStream at current position
    bool Write(const T &record)
    {
        _file_stream.write((char*)&record, sizeof(record));
        _record_count++;
        return true;
    }

    // Output stream to FileStream operator
    ostream &operator << (const T &record)
    {
        _file_stream.write((char*)&record, sizeof(record));
        _record_count++;
        return _file_stream;
    }

    // Write Ts to FileStream at current position (can call multiple times with less than _record_count)
    bool Write(const vector<T> &records)
    {
        for (vector<T>::const_iterator vi = records.begin(); vi != records.end(); ++vi)
        {
            T &record = *vi;
            _file_stream.write((char*)&record, sizeof(record));
            _record_count++;
        }
        return true;
    }

    // Show Header if start_record == 0, and show all records if num_records == 0, else show num_records
    string ToString(const unsigned long long &start_record = 0, const unsigned long long &num_records = 0, const string &separator = " ")
    {
        ostringstream os;
        if (ReadFooter() > 0 && OpenRead())
        {
            if (start_record == 0 && ReadHeader())
            {
                os << setprecision(8) << _version << separator << _file_magic << separator << endl;
            }
            else SeekReadRecord(start_record);
            unsigned long long disp_records = num_records == 0 ? _record_count : num_records;
            if (start_record + disp_records > _record_count) disp_records = _record_count - start_record;
            for (unsigned long long i = 0; i < disp_records; ++i)
            {
                T record;
                if (Read(record))
                {
                    os << record.ToString(separator) << endl;
                }
            }
        }
        else cerr << "ToString: File [" << _file_path << "] could not be opened" << endl;
        return os.str();
    }

    // Writes out string version of binary file in _file_path to out_file_path, returning true on success
    bool WriteTextFile(const string &out_file_path, const string &separator = " ")
    {
        if (ReadFooter() > 0 && OpenRead())
        {
            fstream fs;
            fs.open(out_file_path, ios::out | ios::trunc);
            if (fs.is_open())
            {
                if (ReadHeader())
                {
                    fs << "Version " << setprecision(8) << _version << "  Magic " << _file_magic << "  Records " << _record_count << endl;
                    for (unsigned long long i = 0; i < _record_count; ++i)
                    {
                        T record;
                        if (Read(record))
                        {
                            fs << record.ToString(separator) << endl;
                        }
                    }
                    fs.close();
                    Close();
                    return true;
                }
                else
                {
                    cerr << "WriteTextFile: Input binary file [" << _file_path << "] problem reading header" << endl;
                    return false;
                }
            }
            else
            {
                cerr << "WriteTextFile: Output text file [" << out_file_path << "] could not be created" << endl;
                return false;
            }
        }
        else
        {
            cerr << "WriteTextFile: Input binary file [" << _file_path << "] does not exist or has invalid footer. Records " << _record_count << endl;
            return false;
        }
    }
};
