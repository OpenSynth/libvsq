/**
 * Sequence.hpp
 * Copyright © 2012 kbinani
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
#ifndef __Sequence_hpp__
#define __Sequence_hpp__

#include "vsqglobal.hpp"
#include "Track.hpp"
#include "TempoList.hpp"
#include "TimesigList.hpp"
#include "OutputStream.hpp"
#include "MidiEvent.hpp"
#include "CP932Converter.hpp"
#include "NrpnEvent.hpp"
#include "BitConverter.hpp"
#include <vector>
#include <string.h>

VSQ_BEGIN_NAMESPACE

using namespace std;

/**
 * @brief VSQ ファイルのシーケンスを保持するクラス
 * @todo PreMeasureの取得方法が２通り以上ある。Sequence::master.preMeasureとSequence::getPreMeasure
 */
class Sequence{
public:
    /**
     * @brief テンポ情報を保持したテーブル
     */
    TempoList tempoList;

    /**
     * @brief 拍子情報を保持したテーブル
     */
    TimesigList timesigList;

    /**
     * @brief プリメジャーを保持する
     */
    Master master;

    /**
     * @brief ミキサー情報
     */
    Mixer mixer;

    /**
     * @brief シーケンスに付属するタグ情報
     */
    string tag;

protected:
    /**
     * @brief トラックのリスト
     */
    vector<Track> _track;

private:
    /**
     * @brief テンポが省略された際の、基準となるテンポ値
     */
    static const int baseTempo = 500000;

    /**
     * @brief 四分音符 1 個あたりの Tick 数
     */
    static const int _tickPerQuarter = 480;

    /**
     * @brief Tick 単位の曲の長さ
     */
    tick_t _totalClocks;

public:
    /**
     * @brief 初期化を行う
     */
    explicit Sequence(){
        init( "", 1, 4, 4, baseTempo );
    }

    /**
     * @brief 初期化を行う
     * @param singer (string) 歌手名
     * @param preMeasure (int) 小節単位のプリメジャー
     * @param numerator (int) 拍子の分子の値
     * @param denominator (int) 拍子の分母の値
     * @param tempo (int) テンポ値。四分音符の長さのマイクロ秒単位の長さ
     */
    explicit Sequence( const string &singer, int preMeasure, int numerator, int denominator, int tempo ){
        init( singer, preMeasure, numerator, denominator, tempo );
    }

    /**
     * @brief コピーを作成する
     * @return (Sequence) オブジェクトのコピー
     */
    Sequence clone() const{
        Sequence ret( "Miku", 1, 4, 4, baseTempo );
        ret._track.clear();
        for (int i = 0; i < _track.size(); i++) {
            ret._track.push_back(_track[i].clone());
        }

        ret.tempoList = TempoList();
        for( int i = 0; i < tempoList.size(); i++ ){
            ret.tempoList.push( tempoList.get( i ).clone() );
        }

        TimesigList copy = timesigList;
        for( int i = 0; i < copy.size(); i++ ){
            ret.timesigList.push( copy.get( i ).clone() );
        }

        ret._totalClocks = _totalClocks;
        ret.master = master.clone();
        ret.mixer = mixer.clone();
        return ret;
    }

    /**
     * @brief Get an instance of Track.
     */
    const VSQ_NS::Track *track(int trackIndex) const{
        return &_track[trackIndex];
    }

    /**
     * @brief Get an instance of Track.
     */
    VSQ_NS::Track *track(int trackIndex) {
        return &_track[trackIndex];
    }

    const std::vector<VSQ_NS::Track> *tracks()const {
        return &_track;
    }

    std::vector<VSQ_NS::Track> *tracks() {
        return &_track;
    }

    /**
     * @brief テンポが一つも指定されていない場合の、基本テンポ値を取得する
     * @return (int) テンポ値。四分音符の長さのマイクロ秒単位の長さ
     */
    int getBaseTempo() const{
        return Sequence::baseTempo;
    }

    /**
     * @brief Tick 単位の曲の長さを取得する
     * シーケンスに変更を加えた場合、<code><a href="#updateTotalClocks">updateTotalClocks</a></code> を呼んでからこのメソッドを呼ぶこと
     * @return (int) Tick 単位の曲の長さ
     */
    tick_t getTotalClocks() const{
        return _totalClocks;
    }

    /**
     * @brief プリメジャー値を取得する
     * @return (int) 小節単位のプリメジャー長さ
     */
    int getPreMeasure() const{
        return master.preMeasure;
    }

    /**
     * @brief Tick 単位のプリメジャー部分の長さを取得する
     * @return (int) Tick 単位のプリメジャー長さ
     */
    tick_t getPreMeasureClocks() const{
        return _calculatePreMeasureInClock();
    }

    /**
     * @brief 四分音符あたりの Tick 数を取得する
     * @return (int) 四分音符一つあたりの Tick 数
     */
    tick_t getTickPerQuarter() const{
        return _tickPerQuarter;
    }

    /**
     * @brief カーブ名のリストを取得する
     * @return カーブ名のリスト
     */
    static const vector<string> getCurveNameList(){
        vector<string> result;
        result.push_back( "VEL" );
        result.push_back( "DYN" );
        result.push_back( "BRE" );
        result.push_back( "BRI" );
        result.push_back( "CLE" );
        result.push_back( "OPE" );
        result.push_back( "GEN" );
        result.push_back( "POR" );
        result.push_back( "PIT" );
        result.push_back( "PBS" );
        return result;
    }

    /**
     * @brief totalClock の値を更新する
     */
    void updateTotalClocks(){
        tick_t max = getPreMeasureClocks();
        vector<string> curveNameList = getCurveNameList();
        for (int i = 0; i < _track.size(); i++) {
            Track *track = &(this->_track[i]);
            int numEvents = track->events()->size();
            if( 0 < numEvents ){
                const Event *lastItem = track->events()->get( numEvents - 1 );
                max = ::max( max, lastItem->clock + lastItem->getLength() );
            }
            for( int j = 0; j < curveNameList.size(); j++ ){
                string vct = curveNameList[j];
                const BPList *list = track->curve(vct);
                if( list ){
                    int size = list->size();
                    if( size > 0 ){
                        tick_t last_key = list->getKeyClock( size - 1 );
                        max = ::max( max, last_key );
                    }
                }
            }
        }
        _totalClocks = max;
    }


    //TODO:not implemented yet; getMaximumNoteLengthAt method
    /*
        --
        -- 指定したクロックにおける、音符長さ(ゲートタイム単位)の最大値を調べます
        -- @param clock [int]
        -- @return [int]
        function this:getMaximumNoteLengthAt( clock )
            local secAtStart = self.tempoList:getSecFromClock( clock );
            local secAtEnd = secAtStart + Id.MAX_NOTE_MILLISEC_LENGTH / 1000.0;
            local clockAtEnd = math.floor( self.tempoList:getClockFromSec( secAtEnd ) - 1 );
            secAtEnd = self.tempoList:getSecFromClock( clockAtEnd );
            while ( math.floor( secAtEnd * 1000.0 ) - math.floor( secAtStart * 1000.0 ) <= Id.MAX_NOTE_MILLISEC_LENGTH )do
                clockAtEnd = clockAtEnd + 1;
                secAtEnd = self.tempoList:getSecFromClock( clockAtEnd );
            end
            clockAtEnd = clockAtEnd - 1;
            return clockAtEnd - clock;
        end
*/

/*
        --
        --  指定したトラックのデータから，NRPNを作成します
        -- @param vsq [VsqFile]
        -- @param track [int]
        -- @param msPreSend [int]
        -- @param clock_start [int]
        -- @param clock_end [int]
        -- @return [VsqNrpn[] ]
        function Sequence._generateNRPN_5( vsq, track, msPreSend, clock_start, clock_end )
            local temp = vsq:clone();
            temp.removePart( clock_end, vsq.TotalClocks );
            if( 0 < clock_start )then
                temp.removePart( 0, clock_start );
            end
            temp.Master.PreMeasure = 1;
            --temp.m_premeasure_clocks = temp.getClockFromBarCount( 1 );
            local ret = Sequence._generateNRPN_3( temp, track, msPreSend );
            temp = nil;
            return ret;
        end
*/

private:
    /**
     * @brief プリメジャーの Tick 単位の長さを計算する
     * @return (int) Tick 単位のプリメジャー長さ
     */
    tick_t _calculatePreMeasureInClock() const{
        int pre_measure = master.preMeasure;
        int last_bar_count = timesigList.get( 0 ).barCount;
        tick_t last_clock = timesigList.get( 0 ).getClock();
        int last_denominator = timesigList.get( 0 ).denominator;
        int last_numerator = timesigList.get( 0 ).numerator;
        for( int i = 1; i < timesigList.size(); i++ ){
            if( timesigList.get( i ).barCount >= pre_measure ){
                break;
            }else{
                last_bar_count = timesigList.get( i ).barCount;
                last_clock = timesigList.get( i ).getClock();
                last_denominator = timesigList.get( i ).denominator;
                last_numerator = timesigList.get( i ).numerator;
            }
        }

        int remained = pre_measure - last_bar_count;// プリメジャーの終わりまでの残り小節数
        return last_clock + (int)::floor( remained * last_numerator * 480 * 4 / (double)last_denominator );
    }

    /**
     * @brief 初期化を行う
     * @param singer (string) 歌手名
     * @param preMeasure (int) 小節単位のプリメジャー
     * @param numerator (int) 拍子の分子の値
     * @param denominator (int) 拍子の分母の値
     * @param tempo (int) テンポ値。四分音符の長さのマイクロ秒単位の長さ
     */
    void init( const string &singer, int preMeasure, int numerator, int denominator, int tempo ){
        _totalClocks = preMeasure * 480 * 4 / denominator * numerator;
        _track.push_back(Track("Voice1", singer));
        master = Master( preMeasure );
        mixer = Mixer( 0, 0, 0, 0 );
        mixer.slave.push_back( MixerItem( 0, 0, 0, 0 ) );
        timesigList.push( Timesig( numerator, denominator, 0 ) );
        tempoList.push( Tempo( 0, tempo ) );
    }
};

VSQ_END_NAMESPACE

#endif
