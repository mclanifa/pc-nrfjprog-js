/* Copyright (c) 2015 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Use in source and binary forms, redistribution in binary form only, with
 * or without modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 2. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 3. This software, with or without modification, must only be used with a Nordic
 *    Semiconductor ASA integrated circuit.
 *
 * 4. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef NRFJPROG_BATONS_H
#define NRFJPROG_BATONS_H

#include "highlevel_common.h"
#include "highlevel_helpers.h"
#include <memory>
#include <mutex>
#include <sstream>

class Baton
{
  public:
    explicit Baton(const std::string _name, const uint32_t _returnParameterCount,
                   const bool _mayHaveProgressCallback, const probe_type_t _probeType = DEBUG_PROBE, Probe_handle_t _probe = nullptr)
        : returnParameterCount(_returnParameterCount)
        , name(_name)
        , mayHaveProgressCallback(_mayHaveProgressCallback)
        , serialNumber(0)
        , coProcessor(CP_APPLICATION)
        , result(JsSuccess)
        , probeType(_probeType)
        , probe(_probe)
        , lowlevelError(SUCCESS)
        , cpuNeedsReset(false)
    {
        req       = std::make_unique<uv_work_t>();
        req->data = static_cast<void *>(this);
    }

    virtual ~Baton()
    {
        req.reset();
        callback.reset();
    }

    const int32_t returnParameterCount;
    const std::string name;
    const bool mayHaveProgressCallback;

    uint32_t serialNumber;
    coprocessor_t coProcessor;
    uint32_t result;
    probe_type_t probeType;
    Probe_handle_t probe;
    nrfjprogdll_err_t lowlevelError;
    bool cpuNeedsReset;

    std::chrono::high_resolution_clock::time_point functionStart;

    std::unique_ptr<uv_work_t> req;
    std::unique_ptr<Nan::Callback> callback;

    execute_function_t executeFunction;
    return_function_t returnFunction;

    static std::timed_mutex executionMutex;
};

class BatonNeedsReset : public Baton
{
  public:
    BatonNeedsReset(const std::string _name, const uint32_t _returnParameterCount,
                    const bool _mayHaveProgressCallback,
                    const probe_type_t _probeType = DEBUG_PROBE,
                    Probe_handle_t _probe = nullptr)
          : Baton(_name, _returnParameterCount, _mayHaveProgressCallback, _probeType, _probe)
      {
        cpuNeedsReset = true;
      }
};

class GetLibraryVersionBaton : public Baton
{
  public:
    GetLibraryVersionBaton()
        : Baton("get library version", 1, false)
    {}
    uint32_t major;
    uint32_t minor;
    uint32_t revision;
};

class GetConnectedDevicesBaton : public Baton
{
  public:
    GetConnectedDevicesBaton()
        : Baton("get connected devices", 1, false)
    {}
    std::vector<std::unique_ptr<ProbeDetails>> probes;
};

class GetSerialNumbersBaton : public Baton
{
  public:
    GetSerialNumbersBaton()
        : Baton("get serial numbers", 1, false)
    {}
    std::vector<uint32_t> serialNumbers;
};

class GetDeviceInfoBaton : public Baton
{
  public:
    GetDeviceInfoBaton()
        : Baton("get device info", 1, false)
    {}
    uint32_t serialNumber;
    device_info_t deviceInfo;
};

class GetProbeInfoBaton : public Baton
{
  public:
    GetProbeInfoBaton()
        : Baton("get probe info", 1, false)
    {}
    probe_info_t probeInfo;
};

class GetLibraryInfoBaton : public Baton
{
  public:
    GetLibraryInfoBaton()
        : Baton("get library info", 1, false)
    {}
    library_info_t libraryInfo;
};

class GetDeviceVersionBaton : public Baton
{
  public:
    GetDeviceVersionBaton()
        : Baton("get device version", 1, false)
    {}
    uint32_t serialNumber;
    device_version_t deviceVersion;
};

class ReadBaton : public BatonNeedsReset
{
  public:
    ReadBaton()
        : BatonNeedsReset("read", 1, false)
    {}

    uint32_t address;
    uint32_t length;
    std::vector<uint8_t> data;
};

class ReadU32Baton : public BatonNeedsReset
{
  public:
    ReadU32Baton()
        : BatonNeedsReset("read u32", 1, false)
    {}
    uint32_t address;
    uint32_t length;
    uint32_t data;
};

class ProgramBaton : public BatonNeedsReset
{
  public:
    ProgramBaton()
        : BatonNeedsReset("program", 0, true)
    {}
    std::string file;
    std::string filename;
    program_options_t options;
    input_format_t inputFormat;
};

class ProgramDFUBaton : public BatonNeedsReset
{
  public:
    ProgramDFUBaton()
        : BatonNeedsReset("program", 0, true, DFU_PROBE)
    {}
    std::string filename;
};

class ProgramMcuBootDFUBaton : public Baton
{
  public:
    ProgramMcuBootDFUBaton()
        : Baton("program", 0, true, MCUBOOT_PROBE)
    {}
    std::string filename;
    std::string uart;
    uint32_t baudRate;
    uint32_t responseTimeout;
};

class ProgramModemUartDFUBaton : public Baton
{
  public:
    ProgramModemUartDFUBaton()
        : Baton("program", 0, true, MODEMUARTDFU_PROBE)
    {}
    std::string filename;
    std::string uart;
    uint32_t baudRate;
    uint32_t responseTimeout;
};

class VerifyBaton : public BatonNeedsReset
{
  public:
    VerifyBaton()
        : BatonNeedsReset("verify", 0, true)
    {}
    std::string filename;
};

class ReadToFileBaton : public BatonNeedsReset
{
  public:
    ReadToFileBaton()
        : BatonNeedsReset("read to file", 0, true)
    {}
    std::string filename;
    read_options_t options;
};

class EraseBaton : public BatonNeedsReset
{
  public:
    EraseBaton()
        : BatonNeedsReset("erase", 0, true)
    {}
    erase_action_t erase_mode;
    uint32_t start_address;
    uint32_t end_address;
};

class RecoverBaton : public BatonNeedsReset
{
  public:
    RecoverBaton()
        : BatonNeedsReset("recover", 0, true)
    {}
};

class ResetBaton : public Baton
{
  public:
    ResetBaton()
        : Baton("reset", 0, false)
    {}
};

class WriteBaton : public BatonNeedsReset
{
  public:
    WriteBaton()
        : BatonNeedsReset("write", 0, false)
    {}
    uint32_t address;
    std::vector<uint8_t> data;
    uint32_t length;
};

class WriteU32Baton : public BatonNeedsReset
{
  public:
    WriteU32Baton()
        : BatonNeedsReset("write u32", 0, false)
    {}
    uint32_t address;
    uint32_t data;
};

class OpenBaton : public Baton
{
  public:
    OpenBaton()
        : Baton("open device long term", 0, false)
    {}
};

class CloseBaton : public BatonNeedsReset
{
  public:
    CloseBaton()
        : BatonNeedsReset("close opened device", 0, false)
    {}
};

class RTTStartBaton : public Baton
{
  public:
    RTTStartBaton()
        : Baton("start rtt", 2, false)
    {}
    std::string toString()
    {
        std::stringstream stream;

        stream << "Parameters:" << std::endl;
        stream << "Serialnumber: " << serialNumber << std::endl;
        stream << "Has Controlblock: " << (hasControlBlockLocation ? "true" : "false") << std::endl;
        stream << "Controlblock location: " << controlBlockLocation << std::endl;

        return stream.str();
    }

    uint32_t serialNumber;
    bool hasControlBlockLocation;
    bool foundControlBlock;
    uint32_t controlBlockLocation;

    uint32_t clockSpeed;
    device_family_t family;
    std::string jlinkarmlocation;

    bool foundChannelInformation;
    std::vector<std::unique_ptr<ChannelInfo>> upChannelInfo;
    std::vector<std::unique_ptr<ChannelInfo>> downChannelInfo;
};

class RTTStopBaton : public Baton
{
  public:
    RTTStopBaton()
        : Baton("stop rtt", 0, false)
    {}
    std::string toString()
    {
        return "No parameters";
    }

    bool rttNotStarted;
};

class RTTReadBaton : public Baton
{
  public:
    RTTReadBaton()
        : Baton("rtt read", 3, false)
    {}
    std::string toString()
    {
        std::stringstream stream;

        stream << "Parameters:" << std::endl;
        stream << "ChanneldIndex: " << channelIndex << std::endl;
        stream << "Length wanted: " << length << std::endl;
        stream << "RTT not started: " << rttNotStarted;

        return stream.str();
    }

    uint32_t channelIndex;
    uint32_t length;
    std::vector<char> data;

    bool rttNotStarted;
};

class RTTWriteBaton : public Baton
{
  public:
    RTTWriteBaton()
        : Baton("rtt write", 2, false)
    {}
    std::string toString()
    {
        std::stringstream stream;

        stream << "Parameters:" << std::endl;
        stream << "ChanneldIndex: " << channelIndex << std::endl;
        stream << "Length wanted: " << length << std::endl;
        stream << "Data" << data.data() << std::endl;

        return stream.str();
    }

    uint32_t channelIndex;
    uint32_t length;
    std::vector<char> data;

    bool rttNotStarted;
};

#endif
