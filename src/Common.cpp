/**
 * Common.cpp
 * Copyright © 2014 kbinani
 *
 * This file is part of libvsq.
 *
 * libvsq is free software; you can redistribute it and/or
 * modify it under the terms of the BSD License.
 *
 * libvsq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "../include/libvsq/Common.hpp"
#include "../include/libvsq/TextStream.hpp"
#include "../include/libvsq/StringUtil.hpp"
#include <vector>
#include <sstream>

LIBVSQ_BEGIN_NAMESPACE

Common::Common()
{
	init();
}

Common::Common(TextStream& stream, std::string& lastLine)
{
	init();
	this->version = "";
	this->name = "";
	this->color = "0,0,0";
	this->dynamicsMode = DynamicsMode::STANDARD;
	this->_playMode = PlayMode::PLAY_WITH_SYNTH;
	lastLine = stream.readLine();
	while (lastLine.find("[") != 0) {
		auto spl = StringUtil::explode("=", lastLine);
		std::string search = spl[0];
		if (search == "Version") {
			this->version = spl[1];
		} else if (search == "Name") {
			this->name = spl[1];
		} else if (search == "Color") {
			this->color = spl[1];
		} else if (search == "DynamicsMode") {
			this->dynamicsMode = static_cast<DynamicsMode>(StringUtil::parseInt<int>(spl[1]));
		} else if (search == "PlayMode") {
			this->_playMode = (PlayMode)StringUtil::parseInt<int>(spl[1]);
		}
		if (!stream.ready()) {
			break;
		}
		lastLine = stream.readLine();
	}
}

Common::Common(std::string const& name, int r, int g, int b, DynamicsMode dynamicsMode, PlayMode playMode)
{
	init();
	this->version = "DSB301";
	this->name = name;
	std::ostringstream oss;
	oss << r << "," << g << "," << b;
	this->color = oss.str();
	this->dynamicsMode = dynamicsMode;
	this->_playMode = playMode;
}

Common Common::clone() const
{
	auto spl = StringUtil::explode(",", this->color);
	int r = StringUtil::parseInt<int>(spl[0]);
	int g = StringUtil::parseInt<int>(spl[1]);
	int b = StringUtil::parseInt<int>(spl[2]);
	Common result(this->name, r, g, b, this->dynamicsMode, this->_playMode);
	result.version = this->version;
	result._lastPlayMode = this->_lastPlayMode;
	return result;
}

PlayMode Common::playMode() const
{
	return _playMode;
}

void Common::playMode(PlayMode mode)
{
	_playMode = mode;
	if (mode != PlayMode::OFF) {
		_lastPlayMode = mode;
	}
}

PlayMode Common::lastPlayMode() const
{
	return _lastPlayMode;
}

void Common::write(TextStream& stream) const
{
	stream.writeLine("[Common]");
	stream.writeLine("Version=" + this->version);
	stream.writeLine("Name=" + this->name);
	stream.writeLine("Color=" + this->color);
	std::ostringstream oss;
	oss << "DynamicsMode=" << (int)this->dynamicsMode;
	stream.writeLine(oss.str());

	oss.str("");
	oss << "PlayMode=" << (int)this->_playMode;
	stream.writeLine(oss.str());
}

void Common::init()
{
	this->version = "DSB301";
	this->name = "Miku";
	this->dynamicsMode = DynamicsMode::EXPERT;
	this->_playMode = PlayMode::PLAY_WITH_SYNTH;
	this->color = "179,181,123";
	this->_lastPlayMode = PlayMode::PLAY_WITH_SYNTH;
}

LIBVSQ_END_NAMESPACE
