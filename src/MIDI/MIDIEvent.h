/*
 * Copyright (C) 2014 Michael Gregorius
 *
 * This file is part of ode2joy.
 *
 * ode2joy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ode2joy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ode2joy.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MIDIEVENT_H_
#define MIDIEVENT_H_

#include <cstdint>
#include <cstddef>


class MIDIEvent
{
public:
    enum Status
    {
        NoteOn = 0x90,
        NoteOff = 0x80,
        PolyphonicAftertouch = 0xA0,
        ControlChange = 0xB0,
        ProgramChange = 0xC0,
        ChannelAftertouch = 0xD0,
        PitchBending = 0xE0
    };

public:
    MIDIEvent();
    ~MIDIEvent();

    inline Status getStatus() const { return static_cast<Status>(m_status & 0xf0); }
    inline void setStatus(Status status) { m_status = (static_cast<uint8_t>(status) & 0xf0) | (m_status & 0x0f); }

    inline uint8_t getChannel() const { return m_status & 0x0f; }
    inline void setChannel(uint8_t channel) { m_status = (m_status & 0xf0) | (channel & 0x0f); }

    inline uint8_t getNote() const { return getData(m_data1); }
    inline void setNote(uint8_t note) { setData(m_data1, note); }

    inline uint8_t getVelocity() const { return getData(m_data2); }
    inline void setVelocity(uint8_t velocity) { setData(m_data2, velocity); }

    inline uint8_t getPolyphonicAftertouch() const { return getData(m_data2); }
    inline void setPolyphonicAftertouch(uint8_t aftertouch) { setData(m_data2, aftertouch); }

    inline uint8_t getController() const { return getData(m_data1); }
    inline void setController(uint8_t controller) { setData(m_data1, controller); }

    inline uint8_t getControllerValue() const { return getData(m_data2); }
    inline void setControllerValue(uint8_t controllerValue) { setData(m_data2, controllerValue); }

    inline uint8_t getProgramChangeInstrument() const { return getData(m_data1); }
    inline void setProgramChangeInstrument(uint8_t programChangeInstrument) { setData(m_data1, programChangeInstrument); }

    inline uint8_t getChannelAftertouch() const { return getData(m_data1); }
    inline void setChannelAftertouch(uint8_t channelAftertouch) { setData(m_data1, channelAftertouch); }

    inline uint16_t getPitchBend() const { return static_cast<uint16_t>(m_data2) * 128 + static_cast<uint16_t>(m_data1); }
    inline void setPitchBend(uint16_t pitchBend)
    {
        uint16_t sanitizedPitchBend = pitchBend & 0x3FFF;
        m_data1 = static_cast<uint8_t>(sanitizedPitchBend & 0x007F);
        m_data2 = static_cast<uint8_t>(sanitizedPitchBend >> 7);
    }

    inline uint8_t getStatusByte() const { return m_status; }
    inline uint8_t getData1Byte() const { return m_data1; }
    inline uint8_t getData2Byte() const { return m_data2; }

    // Complete MMC command message (F0 7F <device> 06 <command> F7).
    static MIDIEvent makeMmc(uint8_t deviceId, uint8_t command);

    inline bool isSysEx() const { return m_sysExSize > 0; }
    inline uint8_t const *getSysExBytes() const { return m_sysEx; }
    inline size_t getSysExSize() const { return m_sysExSize; }

private:
    inline uint8_t getData(uint8_t const & data) const { return data & 0x7F; }

    /**
     * @brief setData Convenience method to set the given data byte to the given value.
     *
     * Ensures that the data byte starts with a 0.
     * @param data Reference to the data byte to set.
     * @param value The value to set the data byte to.
     */
    inline void setData(uint8_t & data, uint8_t value) { data = value & 0x7F; }

private:
    uint8_t m_status; // Always starts with a 1
    uint8_t m_data1; // Always starts with a 0
    uint8_t m_data2; // Always starts with a 0
    uint8_t m_sysEx[6];
    size_t m_sysExSize; // 0 for non-SysEx events
};

#endif /* MIDIEVENT_H_ */
