#ifndef jdEnvelope_hpp
#define jdEnvelope_hpp

//#include <stdio.h>
//#include <vector>
//#include <iostream>
//#include <math.h>

/* 
 To Do:
    Implement Easings enum;
    looping until released;
 */
#include "jd_audio_math.h"

template <typename FloatType>
class EnvelopeReader {
public:
    
    void setIncrementRate (FloatType aIncrementRate)
    {
        mIncrementRate = aIncrementRate;
    }
    
    void prepareAction(FloatType aStarting,
                       FloatType aTarget,
                       FloatType aTimeSeconds,
                       FloatType aCurve,
                       bool aIsSustainType  )
    {
        starting = aStarting;
        target = aTarget;
        timeFrames = aTimeSeconds * mIncrementRate;
        amount = target - starting;
        calculateIncrement();
        curve = aCurve;
        current = 0.;
        completionTriggered = false;
        releaseTriggered = false;
        isSustainType = aIsSustainType;
    }
    
    void calculateIncrement ()
    {
        increment = 1. / timeFrames;
    }
    
    void updateAction()
    {
        if (isSustainType)
        {
            if (releaseTriggered)
                completionTriggered = true;
        }
        if ((current + increment) >= 1. )
        {
            current = 1.;
            if (isSustainType)
            {
                if (releaseTriggered)
                    completionTriggered = true;
            }
            else
            {
                completionTriggered = true;
            }
        }
        else { current += increment; }
    }
    
    void release() {  releaseTriggered = true; }
    
    void endAction ()
    {
        if (completionTriggered)
        {
            completionTriggered = false;
            releaseTriggered = false;
        }
    }
    
    void clear()
    {
        amount = starting = current = target = 0.;
        curve = 1.;
    }
    
    FloatType const value() {  return (powf(current, curve) * amount) + starting; }
    
    FloatType amount {0.};
    FloatType starting {0.};
    FloatType increment {0.};
    FloatType current {0.};
    FloatType target {0.};
    FloatType timeFrames {0.};
    FloatType curve { 1.};
    
    bool isChanging;
    bool completionTriggered;
    bool isSustainType;
    bool releaseTriggered;
    
    FloatType mIncrementRate {44100.};
};
    

template <typename FloatType>
class Envelope {
public:
    
    enum EnvelopeState : bool
    {
        Closed = false,
        Open = true
    };
    
    void init (double sampleRate, int blockSize)
    {
        setIncrementRate(sampleRate);
        triggers.resize(blockSize);
        releases.resize(blockSize);
    }
    
    void make(const std::vector<FloatType> &&aLevels,
              const std::vector<FloatType> &&aTimes,
              const std::vector<FloatType> &&aCurves,
              const FloatType aMul = 1.,
              const FloatType aAdd = 0.     )
    {
        
        //Throw Exceoption if sizes are not right
        levels = aLevels;
        times = aTimes;
        curves = aCurves;
        mul = aMul;
        add = aAdd;
        
        index = 0;
        sustainIndex = 0;
        prepareAction();
        gate = EnvelopeState::Closed;
        triggers.clear();
        releases.clear();
    }
    //ADSR
    void adsr (FloatType aAttackTime = 0.1,
               FloatType aDecayTime = 0.1,
               FloatType aSustainLevel = 0.5,
               FloatType aReleaseTime = 0.5,
               FloatType aCurve = 2.,
               FloatType aMul = 1.,
               FloatType aAdd = 0.  )
    {
        make(
             {0, 1., aSustainLevel, 0},
             {aAttackTime, aDecayTime, aReleaseTime},
             {aCurve, aCurve, aCurve},
             aMul,
             aAdd  );
        setSustainNodes({1});
    }
    void setAttackTime (FloatType newAttackTime)
    {
        times[0] = newAttackTime;
    }
    void setDecayTime (FloatType newDecayTime)
    {
        times[1] = newDecayTime;
    }
    void setReleaseTime (FloatType newReleaseTime)
    {
        times[2] = newReleaseTime;
    }
    
    void trigger () {
        if (gate)
            reset();
        gate = EnvelopeState::Open;
        //Is this the best place?
        if (isFirstPoint()) updateAction();// so dont get inf curve on first sample
    }
    void triggerOnSample(size_t sampleIndex) { triggers[sampleIndex] = true; }
    void release () { envReader.release(); }
    void releaseOnSample(size_t sampleIndex) { releases[sampleIndex] = true; }
    
    void setSustainNodes (std::vector<int> const aSustainNodeIndices)
    {
        sustainIndices = aSustainNodeIndices;
        prepareAction();
    }
    
    void clear() {
        levels.clear();
        times.clear();
        curves.clear();
        
        envReader.clear();
        
        triggers.clear();
        releases.clear();
        
        mul = 1.;
        add = 0.;
        gate = EnvelopeState::Closed;
        index = 0;
        sustainIndex = 0;
    }

    //ADD/INSERT/REMOVE
    void addFirstNode (FloatType level)
    {
        clear();
        make({level}, {}, {});
    }
    void addNode (FloatType level, FloatType time, FloatType curve, bool isSustainNode = false)
    {
        levels.push_back(level);
        times.push_back(time);
        curves.push_back(curve);
        
        size_t index = times.size();
        if (isSustainNode){
            sustainIndices.push_back(index);
        }
    }
    void insertNode(int index, FloatType level, FloatType time, FloatType curve, bool isSustainNode = false)
    {
        levels.insert(levels.begin() + (index + 1), level);
        times.insert(times.begin() + (index), time);
        curves.insert(curves.begin() + (index), curve);

        if (isSustainNode){
            sustainIndices.push_back((index));
        }
        
    }
    void removeNode(int index)
    {
        levels.erase(levels.begin() + (index + 1));
        times.erase(times.begin() + index);
        curves.erase(curves.begin() + index);
        
        int i = 0;
        for (auto sustainIndex: sustainIndices)
        {
            /*UNTESTED*/
            if (sustainIndex == index) sustainIndices.erase(sustainIndices.begin() + ++i);
        }
    }
    
    void madd (FloatType aMul, FloatType aAdd) {
        mul = aMul;
        add = aAdd;
    }
    void setIncrementRate (FloatType aIncrementRate)
    {
        envReader.setIncrementRate(aIncrementRate);
    }
    void processBlock (const FloatType* input, FloatType *output, int samplesPerBlock)
    {
        for (int sampleIndex = 0; sampleIndex < samplesPerBlock; sampleIndex++)
        {
            updateAction();
            output[sampleIndex] = input[sampleIndex] * value();
        }
    }
    void writeToBlock (FloatType *output, int samplesPerBlock)
    {
        for (int sampleIndex = 0; sampleIndex < samplesPerBlock; sampleIndex++)
        {
            processSample(sampleIndex);
            output[sampleIndex] = value();
        }
    }
    void processSample (int i) {
        if (triggers[i]) { trigger(); triggers[i] = false; }
        if (releases[i]) { release(); releases[i] = false; }
        
        updateAction();
    }
    FloatType const value() { return (envReader.value() * mul + add); }
    void updateAction()
    {
        if (gate)
        {
            envReader.updateAction();
            if (envReader.completionTriggered)
            {
                index++;
                if (shouldPrepareNextAction())
                    prepareAction();
                else
                    reset();
            }
        }
    }
    void reset () {
        //        isChanging = false;
        gate = EnvelopeState::Closed;
        index = 0;
        sustainIndex = 0;
        prepareAction();
    }
    
    std::vector<FloatType> asStdVector(size_t lengthSamples)
    {
        FloatType fullDuration = 0.;
        for (auto const t: times)
            fullDuration+=t;
        std::vector<FloatType> outputVecter (lengthSamples);
        Envelope<FloatType> temp(*this);
        temp.reset();
        temp.setIncrementRate((FloatType)lengthSamples / fullDuration);
        temp.trigger();
        for (auto &ov: outputVecter){
            temp.processSample();
            ov = temp.value();
        }
        return outputVecter;
    }
    
    void applyToBuffer (FloatType* input, size_t lengthSamples)
    {
        FloatType fullDuration = 0.;
        for (auto const t: times)
            fullDuration+=t;
        Envelope<FloatType> temp(*this);
        temp.reset();
        temp.setIncrementRate((FloatType)lengthSamples / fullDuration);
        temp.trigger();
        for (size_t i = 0; i < lengthSamples; i++)
        {
            temp.processSample();
            input[i] *= temp.value();
        }
    }

// private:
    bool isFirstPoint() { return (index == 0); }
    bool shouldPrepareNextAction() {
        return ((levels.size() - index) >= 1) ? true : false;
    }
    void checkIfActionIsSustainType () {
        bool answer = false;
        if (sustainIndex < sustainIndices.size())
        {
            int i = sustainIndices[sustainIndex];
            if (i == index) {
                answer = true;
                sustainIndex++;
            }
        }
        m_actionIsSustainType = answer;
    }
    bool actionIsSustainType()
    { return m_actionIsSustainType;
    }
    void prepareAction()
    {
        if (levels.size() > 0){
            FloatType target = levels[index];
            FloatType time = (index == 0) ? 0. : times[index - 1];
            FloatType curve = (index == 0) ? 1. : curves[index - 1];
            checkIfActionIsSustainType();
            envReader.prepareAction(envReader.value(),
                                    target,
                                    time,
                                    curve,
                                    actionIsSustainType());
        }
    }
    
    std::vector<FloatType>      levels;
    std::vector<FloatType>      times;
    std::vector<FloatType>      curves;
    std::vector<int>            sustainIndices;
    std::vector<FloatType>      triggers;
    std::vector<FloatType>      releases;
    bool m_actionIsSustainType = false;
    
    int                         index;
    int                         sustainIndex;
    EnvelopeReader<FloatType>   envReader;
    FloatType                   mul { 1.0 }, add { 0. };
    SmoothedValue<FloatType>    smoothedVal;
    bool                        gate;
};

#endif /* jdEnvelope_hpp */
