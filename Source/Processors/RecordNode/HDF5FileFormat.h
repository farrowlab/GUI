/*
 ------------------------------------------------------------------

 This file is part of the Open Ephys GUI
 Copyright (C) 2014 Florian Franzen

 ------------------------------------------------------------------

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef HDF5FILEFORMAT_H_INCLUDED
#define HDF5FILEFORMAT_H_INCLUDED

#include "../../../JuceLibraryCode/JuceHeader.h"

class HDF5RecordingData;
namespace H5
{
class DataSet;
class H5File;
class DataType;
}

struct HDF5RecordingInfo
{
    String name;
    int64 start_time;
    uint32 start_sample;
    float sample_rate;
    uint32 bit_depth;
};

class HDF5FileBase
{
public:
    HDF5FileBase();
    virtual ~HDF5FileBase();

    int open();
    void close();
    virtual String getFileName() = 0;
    bool isOpen() const;
    typedef enum DataTypes { U8, U16, U32, U64, I8, I16, I32, I64, F32, STR} DataTypes;

    static H5::DataType getNativeType(DataTypes type);
    static H5::DataType getH5Type(DataTypes type);

protected:

    virtual int createFileStructure() = 0;

    int setAttribute(DataTypes type, void* data, String path, String name);
    int setAttributeStr(String value, String path, String name);
    int createGroup(String path);

    HDF5RecordingData* getDataSet(String path);

    //aliases for createDataSet
    HDF5RecordingData* createDataSet(DataTypes type, int sizeX, int chunkX, String path);
    HDF5RecordingData* createDataSet(DataTypes type, int sizeX, int sizeY, int chunkX, String path);
    HDF5RecordingData* createDataSet(DataTypes type, int sizeX, int sizeY, int sizeZ, int chunkX, String path);
    HDF5RecordingData* createDataSet(DataTypes type, int sizeX, int sizeY, int sizeZ, int chunkX, int chunkY, String path);

    bool readyToOpen;

private:
    //create an extendable dataset
    HDF5RecordingData* createDataSet(DataTypes type, int dimension, int* size, int* chunking, String path);
    int open(bool newfile);
    ScopedPointer<H5::H5File> file;
    bool opened;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HDF5FileBase);
};

class HDF5RecordingData
{
public:
    HDF5RecordingData(H5::DataSet* data);
    ~HDF5RecordingData();

    int writeDataBlock(int xDataSize, HDF5FileBase::DataTypes type, void* data);
    int writeDataBlock(int xDataSize, int yDataSize, HDF5FileBase::DataTypes type, void* data);

    int prepareDataBlock(int xDataSize);
    int writeDataRow(int yPos, int xDataSize, HDF5FileBase::DataTypes type, void* data);

private:
    int xPos;
    int xChunkSize;
    int size[3];
    int dimension;
    int rowXPos;
    int rowDataSize;
    ScopedPointer<H5::DataSet> dSet;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HDF5RecordingData);
};

class KWDFile : public HDF5FileBase
{
public:
    KWDFile(int processorNumber, String basename);
    KWDFile();
    virtual ~KWDFile();
    void initFile(int processorNumber, String basename);
    void startNewRecording(int recordingNumber, int nChannels, HDF5RecordingInfo* info);
    void stopRecording();
    void writeBlockData(int16* data, int nSamples);
    void writeRowData(int16* data, int nSamples);
    String getFileName();

protected:
    int createFileStructure();

private:
    int recordingNumber;
    int nChannels;
    int curChan;
    String filename;
    ScopedPointer<HDF5RecordingData> recdata;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KWDFile);
};

class KWIKFile : public HDF5FileBase
{
public:
    KWIKFile(String basename);
    KWIKFile();
    virtual ~KWIKFile();
    void initFile(String basename);
    void startNewRecording(int recordingNumber, HDF5RecordingInfo* info);
    void stopRecording();
    void writeEvent(int type, uint8 id, uint8 processor, void* data, uint64 timestamp);
    void addKwdFile(String filename);
    void addEventType(String name, DataTypes type, String dataName);
    String getFileName();

protected:
    int createFileStructure();

private:
    int recordingNumber;
    String filename;
    OwnedArray<HDF5RecordingData> timeStamps;
    OwnedArray<HDF5RecordingData> recordings;
    OwnedArray<HDF5RecordingData> eventID;
    OwnedArray<HDF5RecordingData> nodeID;
    OwnedArray<HDF5RecordingData> eventData;
    Array<String> eventNames;
	Array<DataTypes> eventTypes;
	Array<String> eventDataNames;
    int kwdIndex;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KWIKFile);
};

class KWXFile : public HDF5FileBase
{
public:
    KWXFile(String basename);
    KWXFile();
    virtual ~KWXFile();
    void initFile(String basename);
    void startNewRecording(int recordingNumber);
    void stopRecording();
    void addChannelGroup(int nChannels);
    void resetChannels();
    void writeSpike(int groupIndex, int nSamples, const uint16* data, uint64 timestamp);
    String getFileName();

protected:
    int createFileStructure();

private:
    int createChannelGroup(int index);
    int recordingNumber;
    String filename;
    OwnedArray<HDF5RecordingData> spikeArray;
    OwnedArray<HDF5RecordingData> recordingArray;
    OwnedArray<HDF5RecordingData> timeStamps;
    Array<int> channelArray;
    int numElectrodes;
    int16* transformVector;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KWXFile);
};

#endif  // HDF5FILEFORMAT_H_INCLUDED
