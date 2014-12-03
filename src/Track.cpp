/**
 * Track.cpp
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
#include "../include/libvsq/Track.hpp"
#include "../include/libvsq/StringUtil.hpp"

VSQ_BEGIN_NAMESPACE

Track::Track()
{
	this->_initCor("Track1", "Miku");
}

Track::Track(std::string const& name, std::string const& singer)
{
	this->_initCor(name, singer);
}

Track::Track(Track const& value)
{
	value.deepCopy(this);
}

Track& Track::operator = (Track const& value)
{
	value.deepCopy(this);
	return *this;
}

std::string Track::name() const
{
	//if( common == nil ){
	//    return "Master Track";
	//}else{
	return _common.name;
	//}
}

void Track::name(std::string const& value)
{
	_common.name = value;
}

EventListIndexIterator Track::getIndexIterator(EventListIndexIteratorKind iteratorKind) const
{
	return EventListIndexIterator(&this->_events, iteratorKind);
}

/**
	-- このトラックの再生モードを取得します.
	--
	-- @return [int] PlayMode.PlayAfterSynthまたはPlayMode.PlayWithSynth
	function this:getPlayMode()
		if( self.common == nil ){
			return PlayModeEnum.PLAY_WITH_SYNTH;
		}
		if( self.common.lastPlayMode ~= PlayModeEnum.PLAY_AFTER_SYNTH and
			 self.common.lastPlayMode ~= PlayModeEnum.PLAY_WITH_SYNTH ){
			self.common.lastPlayMode = PlayModeEnum.PLAY_WITH_SYNTH;
		}
		return self.common.lastPlayMode;
	}
	*/

/**
	-- このトラックの再生モードを設定します.
	--
	-- @param value [int] PlayMode.PlayAfterSynth, PlayMode.PlayWithSynth, またはPlayMode.Offのいずれかを指定します
	-- @return [void]
	function this:setPlayMode( value )
		if( self.MetaText == nil ) return;
		if( self.common == nil ){
			self.common = Common.new( "Miku", 128, 128, 128, DynamicsModeEnum.EXPERT, value );
			return;
		}
		if( value == PlayModeEnum.OFF ){
			if( self.common.playMode ~= PlayModeEnum.OFF ){
				self.common.lastPlayMode = self.common.playMode;
			}
		else
			self.common.lastPlayMode = value;
		}
		self.common.playMode = value;
	}
	*/

/**
	-- このトラックがレンダリングされるかどうかを取得します.
	--
	-- @return [bool]
	function this:isTrackOn()
		if( self.MetaText == nil ) return true;
		if( self.common == nil ) return true;
		return self.common.playMode ~= PlayModeEnum.OFF;
	}
	*/

/**
	-- このトラックがレンダリングされるかどうかを設定します,
	--
	-- @param value [bool]
	function this:setTrackOn( value )
		if( self.MetaText == nil ) return;
		if( self.common == nil ){
			self.common = Common.new( "Miku", 128, 128, 128, DynamicsModeEnum.EXPERT, value ? PlayModeEnum.PLAY_WITH_SYNTH : PlayModeEnum.OFF );
		}
		if( value ){
			if( self.common.lastPlayMode ~= PlayModeEnum.PLAY_AFTER_SYNTH and
				 self.common.lastPlayMode ~= PlayModeEnum.PLAY_WITH_SYNTH ){
				self.common.lastPlayMode = PlayModeEnum.PLAY_WITH_SYNTH;
			}
			self.common.playMode = self.common.lastPlayMode;
		else
			if( self.common.playMode == PlayModeEnum.PLAY_AFTER_SYNTH or
				 self.common.playMode == PlayModeEnum.PLAY_WITH_SYNTH ){
				self.common.lastPlayMode = self.common.playMode;
			}
			self.common.playMode = PlayModeEnum.OFF;
		}
	}
	*/

/**
	-- このトラックの, 指定したゲートタイムにおけるピッチベンドを取得します. 単位はCentです.
	--
	-- @param clock [int] ピッチベンドを取得するゲートタイム
	-- @return [double]
	function this:getPitchAt( clock )
		local inv2_13 = 1.0 / 8192.0;
		local pit = self.PIT.getValue( clock );
		local pbs = self.PBS.getValue( clock );
		return pit * pbs * inv2_13 * 100.0;
	}
	*/

/**
	-- クレッシェンド, デクレッシェンド, および強弱記号をダイナミクスカーブに反映させます.
	-- この操作によって, ダイナミクスカーブに設定されたデータは全て削除されます.
	-- @return [void]
	function this:reflectDynamics()
		local dyn = self.getCurve( "dyn" );
		dyn.clear();
		local itr;
		for itr = self.getDynamicsEventIterator(); itr.hasNext();
			local item = itr.next();
			local handle = item.IconDynamicsHandle;
			if( handle == nil ){
				continue;
			}
			local clock = item.Clock;
			local length = item.getLength();

			if( handle.isDynaffType() ){
				-- 強弱記号
				dyn.add( clock, handle.getStartDyn() );
			else
				-- クレッシェンド, デクレッシェンド
				local start_dyn = dyn.getValue( clock );

				-- 範囲内のアイテムを削除
				local count = dyn.size();
				local i;
				for i = count - 1; i >= 0; i--
					local c = dyn.getKeyClock( i );
					if( clock <= c and c <= clock + length ){
						dyn.removeElementAt( i );
					}else if( c < clock ){
						break;
					}
				}

				local bplist = handle.getDynBP();
				if( bplist == nil or (bplist ~= nil and bplist.size() <= 0) ){
					-- カーブデータが無い場合
					local a = 0.0;
					if( length > 0 ){
						a = (handle.getEndDyn() - handle.getStartDyn()) / length;
					}
					local last_val = start_dyn;
					local i;
					for i = clock; i < clock + length; i++
						local val = start_dyn + org.kbinani.PortUtil.castToInt( a * (i - clock) );
						if( val < dyn.getMinimum() ){
							val = dyn.getMinimum();
						}else if( dyn.getMaximum() < val ){
							val = dyn.getMaximum();
						}
						if( last_val ~= val ){
							dyn.add( i, val );
							last_val = val;
						}
					}
				else
					-- カーブデータがある場合
					local last_val = handle.getStartDyn();
					local last_clock = clock;
					local bpnum = bplist.size();
					local last = start_dyn;

					-- bplistに指定されている分のデータ点を追加
					local i;
					for i = 0; i < bpnum; i++
						local point = bplist.getElement( i );
						local pointClock = clock + org.kbinani.PortUtil.castToInt( length * point.X );
						if( pointClock <= last_clock ){
							continue;
						}
						local pointValue = point.Y;
						local a = (pointValue - last_val) / (pointClock - last_clock);
						local j;
						for j = last_clock; j <= pointClock; j++
							local val = start_dyn + org.kbinani.PortUtil.castToInt( (j - last_clock) * a );
							if( val < dyn.getMinimum() ){
								val = dyn.getMinimum();
							}else if( dyn.getMaximum() < val ){
								val = dyn.getMaximum();
							}
							if( val ~= last ){
								dyn.add( j, val );
								last = val;
							}
						}
						last_val = point.Y;
						last_clock = pointClock;
					}

					-- bplistの末尾から, clock => clock + lengthまでのデータ点を追加
					local last2 = last;
					if( last_clock < clock + length ){
						local a = (handle.getEndDyn() - last_val) / (clock + length - last_clock);
						local j;
						for j = last_clock; j < clock + length; j++
							local val = last2 + org.kbinani.PortUtil.castToInt( (j - last_clock) * a );
							if( val < dyn.getMinimum() ){
								val = dyn.getMinimum();
							}else if( dyn.getMaximum() < val ){
								val = dyn.getMaximum();
							}
							if( val ~= last ){
								dyn.add( j, val );
								last = val;
							}
						}
					}
				}
			}
		}
	}
	*/

Event const* Track::singerEventAt(tick_t tick) const
{
	Event const* last = nullptr;
	Event::List const& events = this->events();
	EventListIndexIterator itr = getIndexIterator(EventListIndexIteratorKind::SINGER);
	while (itr.hasNext()) {
		int index = itr.next();
		const Event* item = events.get(index);
		if (tick < item->tick) {
			return last;
		}
		last = item;
	}
	return last;
}


/**
	-- このトラックに設定されているイベントを, ゲートタイム順に並べ替えます.
	--
	-- @reutrn [void]
	function this:sortEvent()
		self.events:sort();
	}
	*/

/**
	-- レンダラーを変更します
	--
	-- @param new_renderer [string]
	-- @param singers [Array<VsqID>]
	function this:changeRenderer( new_renderer, singers )
		local default_id = nil;
		local singers_size = #singers;
		if( singers_size <= 0 ){
			default_id = Id.new();
			default_id.type = EventTypeEnum.SINGER;
			local singer_handle = Handle.new( HandleTypeEnum.SINGER );
			singer_handle.IconID = "$0701" + org.kbinani.PortUtil.sprintf( "%04X", 0 );
			singer_handle.ids = "Unknown";
			singer_handle.Index = 0;
			singer_handle.Language = 0;
			singer_handle.setLength( 1 );
			singer_handle.Original = 0;
			singer_handle.Program = 0;
			singer_handle.Caption = "";
			default_id.singerHandle = singer_handle;
		else
			default_id = singers[0];
		}

		local itr;
		for ( itr = self.getSingerEventIterator(); itr.hasNext();
			local ve = itr.next();
			local singer_handle = ve.IconHandle;
			local program = singer_handle.Program;
			local found = false;
			local i;
			for i = 0; i < singers_size; i++
				local id = singers[i];
				if( program == singer_handle.Program ){
					ve.id = id:clone();
					found = true;
					break;
				}
			}
			if( !found ){
				local add = default_id:clone();
				add.IconHandle.Program = program;
				ve.id = add;
			}
		}
		self.common.Version = new_renderer;
	}
	*/

BPList const* Track::curve(std::string const& curveName) const
{
	std::string search = StringUtil::toLower(curveName);
	std::map<std::string, BPList*>::const_iterator index
		= curveNameMap.find(search);
	if (index == curveNameMap.end()) {
		return nullptr;
	} else {
		return index->second;
	}
}

BPList* Track::curve(std::string const& curveName)
{
	std::string search = StringUtil::toLower(curveName);
	std::map<std::string, BPList*>::const_iterator index
		= curveNameMap.find(search);
	if (index == curveNameMap.end()) {
		return nullptr;
	} else {
		return index->second;
	}
}

/*
 * @brief 指定された名前のカーブを設定する
 * @param curve (string) カーブ名
 * @param value (BPList) 設定するカーブ
 */
/*void setCurve( const std::string curve, BPList value ){
	string search = StringUtil::toLower( curve );
	if( search == "bre" ){
		_bre = value;
	}else if( search == "bri" ){
		_bri = value;
	}else if( search == "cle" ){
		_cle = value;
	}else if( search == "dyn" ){
		_dyn = value;
	}else if( search == "gen" ){
		_gen = value;
	}else if( search == "ope" ){
		_ope = value;
	}else if( search == "pbs" ){
		_pbs = value;
	}else if( search == "pit" ){
		_pit = value;
	}else if( search == "por" ){
		_por = value;
	}else if( search == "harmonics" ){
		_harmonics = value;
	}else if( search == "fx2depth" ){
		_fx2depth = value;
	}else if( search == "reso1amp" ){
		_reso1AmpBPList = value;
	}else if( search == "reso1bw" ){
		_reso1BWBPList = value;
	}else if( search == "reso1freq" ){
		_reso1FreqBPList = value;
	}else if( search == "reso2amp" ){
		_reso2AmpBPList = value;
	}else if( search == "reso2bw" ){
		_reso2BWBPList = value;
	}else if( search == "reso2freq" ){
		_reso2FreqBPList = value;
	}else if( search == "reso3amp" ){
		_reso3AmpBPList = value;
	}else if( search == "reso3bw" ){
		_reso3BWBPList = value;
	}else if( search == "reso3freq" ){
		_reso3FreqBPList = value;
	}else if( search == "reso4amp" ){
		_reso4AmpBPList = value;
	}else if( search == "reso4bw" ){
		_reso4BWBPList = value;
	}else if( search == "reso4freq" ){
		_reso4FreqBPList = value;
	}
}*/

Track Track::clone() const
{
	Track res;
	deepCopy(&res);
	return res;
}

/*
	-- 歌詞の文字数を調べます
	-- @return [int]
	function this:getLyricLength()
		local counter = 0;
		local i;
		for i = 0; i < self.events.size(); i++
			if( self.events:getElement( i ).type == EventTypeEnum.NOTE ){
				counter++;
			}
		}
		return counter;
	}
	*/

Event::List& Track::events()
{
	return _events;
}

Event::List const& Track::events() const
{
	return _events;
}

Common& Track::common()
{
	return _common;
}

Common const& Track::common() const
{
	return _common;
}

std::vector<std::string> const* Track::curveNameList() const
{
	static std::vector<std::string> vocaloid1;
	static std::vector<std::string> vocaloid2;
	if (vocaloid1.empty() || vocaloid2.empty()) {
		vocaloid1.clear();
		vocaloid2.clear();

		addCurveNameTo(vocaloid1, vocaloid2, "PIT", true, true);
		addCurveNameTo(vocaloid1, vocaloid2, "PBS", true, true);
		addCurveNameTo(vocaloid1, vocaloid2, "DYN", true, true);
		addCurveNameTo(vocaloid1, vocaloid2, "BRE", true, true);
		addCurveNameTo(vocaloid1, vocaloid2, "BRI", true, true);
		addCurveNameTo(vocaloid1, vocaloid2, "CLE", true, true);

		addCurveNameTo(vocaloid1, vocaloid2, "harmonics", true, false);
		addCurveNameTo(vocaloid1, vocaloid2, "fx2depth", true, false);
		addCurveNameTo(vocaloid1, vocaloid2, "reso1Freq", true, false);
		addCurveNameTo(vocaloid1, vocaloid2, "reso2Freq", true, false);
		addCurveNameTo(vocaloid1, vocaloid2, "reso3Freq", true, false);
		addCurveNameTo(vocaloid1, vocaloid2, "reso4Freq", true, false);
		addCurveNameTo(vocaloid1, vocaloid2, "reso1BW", true, false);
		addCurveNameTo(vocaloid1, vocaloid2, "reso2BW", true, false);
		addCurveNameTo(vocaloid1, vocaloid2, "reso3BW", true, false);
		addCurveNameTo(vocaloid1, vocaloid2, "reso4BW", true, false);
		addCurveNameTo(vocaloid1, vocaloid2, "reso1Amp", true, false);
		addCurveNameTo(vocaloid1, vocaloid2, "reso2Amp", true, false);
		addCurveNameTo(vocaloid1, vocaloid2, "reso3Amp", true, false);
		addCurveNameTo(vocaloid1, vocaloid2, "reso4Amp", true, false);

		addCurveNameTo(vocaloid1, vocaloid2, "GEN", true, true);
		addCurveNameTo(vocaloid1, vocaloid2, "POR", true, true);

		addCurveNameTo(vocaloid1, vocaloid2, "OPE", false, true);
	}

	if (common().version.substr(0, 4) == "DSB2") {
		return &vocaloid1;
	} else {
		return &vocaloid2;
	}
}

std::map<std::string, std::string> Track::getSectionNameMap() const
{
	static std::map<std::string, std::string> result;
	if (result.empty()) {
		result.insert(std::make_pair("[PitchBendBPList]", "pit"));
		result.insert(std::make_pair("[PitchBendSensBPList]", "pbs"));
		result.insert(std::make_pair("[DynamicsBPList]", "dyn"));
		result.insert(std::make_pair("[EpRResidualBPList]", "bre"));
		result.insert(std::make_pair("[EpRESlopeBPList]", "bri"));
		result.insert(std::make_pair("[EpRESlopeDepthBPList]", "cle"));
		result.insert(std::make_pair("[EpRSineBPList]", "harmonics"));
		result.insert(std::make_pair("[VibTremDepthBPList]", "fx2depth"));
		result.insert(std::make_pair("[Reso1FreqBPList]", "reso1Freq"));
		result.insert(std::make_pair("[Reso2FreqBPList]", "reso2Freq"));
		result.insert(std::make_pair("[Reso3FreqBPList]", "reso3Freq"));
		result.insert(std::make_pair("[Reso4FreqBPList]", "reso4Freq"));
		result.insert(std::make_pair("[Reso1BWBPList]", "reso1BW"));
		result.insert(std::make_pair("[Reso2BWBPList]", "reso2BW"));
		result.insert(std::make_pair("[Reso3BWBPList]", "reso3BW"));
		result.insert(std::make_pair("[Reso4BWBPList]", "reso4BW"));
		result.insert(std::make_pair("[Reso1AmpBPList]", "reso1Amp"));
		result.insert(std::make_pair("[Reso2AmpBPList]", "reso2Amp"));
		result.insert(std::make_pair("[Reso3AmpBPList]", "reso3Amp"));
		result.insert(std::make_pair("[Reso4AmpBPList]", "reso4Amp"));
		result.insert(std::make_pair("[GenderFactorBPList]", "gen"));
		result.insert(std::make_pair("[PortamentoTimingBPList]", "por"));
		result.insert(std::make_pair("[OpeningBPList]", "ope"));
	}
	return result;
}

void Track::_initCor(std::string const& name, std::string const& singer)
{
	this->_common = Common(name, 179, 181, 123, DynamicsMode::EXPERT, PlayMode::PLAY_WITH_SYNTH);
	this->_pit = BPList("pit", 0, -8192, 8191);
	this->_pbs = BPList("pbs", 2, 0, 24);
	this->_dyn = BPList("dyn", 64, 0, 127);
	this->_bre = BPList("bre", 0, 0, 127);
	this->_bri = BPList("bri", 64, 0, 127);
	this->_cle = BPList("cle", 0, 0, 127);
	this->_reso1FreqBPList = BPList("reso1freq", 64, 0, 127);
	this->_reso2FreqBPList = BPList("reso2freq", 64, 0, 127);
	this->_reso3FreqBPList = BPList("reso3freq", 64, 0, 127);
	this->_reso4FreqBPList = BPList("reso4freq", 64, 0, 127);
	this->_reso1BWBPList = BPList("reso1bw", 64, 0, 127);
	this->_reso2BWBPList = BPList("reso2bw", 64, 0, 127);
	this->_reso3BWBPList = BPList("reso3bw", 64, 0, 127);
	this->_reso4BWBPList = BPList("reso4bw", 64, 0, 127);
	this->_reso1AmpBPList = BPList("reso1amp", 64, 0, 127);
	this->_reso2AmpBPList = BPList("reso2amp", 64, 0, 127);
	this->_reso3AmpBPList = BPList("reso3amp", 64, 0, 127);
	this->_reso4AmpBPList = BPList("reso4amp", 64, 0, 127);
	this->_harmonics = BPList("harmonics", 64, 0, 127);
	this->_fx2depth = BPList("fx2depth", 64, 0, 127);
	this->_gen = BPList("gen", 64, 0, 127);
	this->_por = BPList("por", 64, 0, 127);
	this->_ope = BPList("ope", 127, 0, 127);

	setupCurveNameMap();

	Event event(0, EventType::SINGER);
	Handle ish(HandleType::SINGER);
	ish.iconId = "$07010000";
	ish.ids = singer;
	ish.original = 0;
	ish.length(1);
	ish.language = 0;
	ish.program = 0;
	event.singerHandle = ish;
	this->_events.add(event);
}

void Track::deepCopy(Track* destination) const
{
	destination->name(name());

	destination->_common = _common.clone();
	destination->_events.clear();
	for (int i = 0; i < _events.size(); i++) {
		const Event* item = _events.get(i);
		destination->_events.add(item->clone(), item->id);
	}
	destination->_pit = _pit.clone();
	destination->_pbs = _pbs.clone();
	destination->_dyn = _dyn.clone();
	destination->_bre = _bre.clone();
	destination->_bri = _bri.clone();
	destination->_cle = _cle.clone();
	destination->_reso1FreqBPList = _reso1FreqBPList.clone();
	destination->_reso2FreqBPList = _reso2FreqBPList.clone();
	destination->_reso3FreqBPList = _reso3FreqBPList.clone();
	destination->_reso4FreqBPList = _reso4FreqBPList.clone();
	destination->_reso1BWBPList = _reso1BWBPList.clone();
	destination->_reso2BWBPList = _reso2BWBPList.clone();
	destination->_reso3BWBPList = _reso3BWBPList.clone();
	destination->_reso4BWBPList = _reso4BWBPList.clone();
	destination->_reso1AmpBPList = _reso1AmpBPList.clone();
	destination->_reso2AmpBPList = _reso2AmpBPList.clone();
	destination->_reso3AmpBPList = _reso3AmpBPList.clone();
	destination->_reso4AmpBPList = _reso4AmpBPList.clone();
	destination->_harmonics = _harmonics.clone();
	destination->_fx2depth = _fx2depth.clone();
	destination->_gen = _gen.clone();
	destination->_por = _por.clone();
	destination->_ope = _ope.clone();

	destination->setupCurveNameMap();
}

void Track::setupCurveNameMap()
{
	curveNameMap.clear();
	curveNameMap.insert(std::make_pair("bre", &_bre));
	curveNameMap.insert(std::make_pair("bri", &_bri));
	curveNameMap.insert(std::make_pair("cle", &_cle));
	curveNameMap.insert(std::make_pair("dyn", &_dyn));
	curveNameMap.insert(std::make_pair("gen", &_gen));
	curveNameMap.insert(std::make_pair("ope", &_ope));
	curveNameMap.insert(std::make_pair("pbs", &_pbs));
	curveNameMap.insert(std::make_pair("pit", &_pit));
	curveNameMap.insert(std::make_pair("por", &_por));
	curveNameMap.insert(std::make_pair("harmonics", &_harmonics));
	curveNameMap.insert(std::make_pair("fx2depth", &_fx2depth));
	curveNameMap.insert(std::make_pair("reso1amp", &_reso1AmpBPList));
	curveNameMap.insert(std::make_pair("reso1bw", &_reso1BWBPList));
	curveNameMap.insert(std::make_pair("reso1freq", &_reso1FreqBPList));
	curveNameMap.insert(std::make_pair("reso2amp", &_reso2AmpBPList));
	curveNameMap.insert(std::make_pair("reso2bw", &_reso2BWBPList));
	curveNameMap.insert(std::make_pair("reso2freq", &_reso2FreqBPList));
	curveNameMap.insert(std::make_pair("reso3amp", &_reso3AmpBPList));
	curveNameMap.insert(std::make_pair("reso3bw", &_reso3BWBPList));
	curveNameMap.insert(std::make_pair("reso3freq", &_reso3FreqBPList));
	curveNameMap.insert(std::make_pair("reso4amp", &_reso4AmpBPList));
	curveNameMap.insert(std::make_pair("reso4bw", &_reso4BWBPList));
	curveNameMap.insert(std::make_pair("reso4freq", &_reso4FreqBPList));
}

void Track::addCurveNameTo(std::vector<std::string>& vocaloid1CurveNameList,
						   std::vector<std::string>& vocaloid2CurveNameList,
						   std::string const& name,
						   bool addToVocaloid1, bool addToVocaloid2) const
{
	if (addToVocaloid1) { vocaloid1CurveNameList.push_back(name); }
	if (addToVocaloid2) { vocaloid2CurveNameList.push_back(name); }
}

VSQ_END_NAMESPACE
