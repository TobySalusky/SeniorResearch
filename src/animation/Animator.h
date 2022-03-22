//
// Created by Tobiathan on 11/29/21.
//

#ifndef SENIORRESEARCH_ANIMATOR_H
#define SENIORRESEARCH_ANIMATOR_H

#include <unordered_map>
#include "../util/Includes.h"
#include "KeyFrameLayer.h"
#include <boost/serialization/access.hpp>
#include <boost/serialization/unordered_map.hpp>

class Animator {
public:

    Animator();

    void SetTime(float time) { currentTime = time; }
    [[nodiscard]] float GetTime() const { return currentTime; }

    bool HasKeyFrameAtTimeOnLayer(float time, Enums::DrawMode drawMode) {
        return keyFrameLayers[drawMode].HasKeyFrameAtTime(time);
    }

	struct SettableFloatKeyFrameLayer {
		KeyFrameLayer<float> layer;
	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			ar & layer;
		}
	};

private:

    float currentTime = 0.0f;
    std::unordered_map<Enums::DrawMode, KeyFrameLayer<std::vector<glm::vec2>>> keyFrameLayers;
    std::unordered_map<std::string, SettableFloatKeyFrameLayer> floatKeyFrameLayers;

	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & currentTime;
		ar & keyFrameLayers;
		ar & floatKeyFrameLayers;
	}

    friend class Timeline;
};


#endif //SENIORRESEARCH_ANIMATOR_H
