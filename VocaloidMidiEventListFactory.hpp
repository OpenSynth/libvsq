/**
 * VocaloidMidiEventListFactory.hpp
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
#ifndef __VocaloidMidiEventListFactory_hpp__
#define __VocaloidMidiEventListFactory_hpp__

#include "vsqglobal.hpp"
#include "NrpnEvent.hpp"
#include "Track.hpp"
#include "TempoList.hpp"
#include "MidiParameterType.hpp"

VSQ_BEGIN_NAMESPACE

using namespace std;

/**
 * @brief
 */
class VocaloidMidiEventListFactory{
protected:
    /**
     * @brief トラックの Expression(DYN) の NRPN リストを作成する
     * @param track 出力するトラック
     * @param tempoList テンポ情報
     * @param preSendMilliseconds ミリ秒単位のプリセンド時間
     * @return NrpnEvent の配列
     */
    static vector<NrpnEvent> generateExpressionNRPN( Track *track, TempoList *tempoList, int preSendMilliseconds ){
        vector<NrpnEvent> ret;
        BPList *dyn = track->getCurve( "DYN" );
        size_t count = dyn->size();
        int lastDelay = 0;
        for( int i = 0; i < count; i++ ){
            tick_t clock = dyn->getKeyClock( i );
            tick_t actualClock;
            int delay;
            _getActualClockAndDelay( tempoList, clock, preSendMilliseconds, &actualClock, &delay );
            if( actualClock >= 0 ){
                if( lastDelay != delay ){
                    int delayMsb, delayLsb;
                    _getMsbAndLsb( delay, &delayMsb, &delayLsb );
                    NrpnEvent delayNrpn( actualClock, MidiParameterType::CC_E_DELAY, delayMsb, delayLsb );
                    ret.push_back( delayNrpn );
                }
                lastDelay = delay;

                NrpnEvent add(
                    actualClock,
                    MidiParameterType::CC_E_EXPRESSION,
                    dyn->getValue( i )
                );
                ret.push_back( add );
            }
        }
        return ret;
    }

    /**
     * @brief トラックの先頭に記録される NRPN を作成する
     * @return NRPNイベント
     */
    static NrpnEvent generateHeaderNRPN(){
        NrpnEvent ret( 0, MidiParameterType::CC_BS_VERSION_AND_DEVICE, 0x00, 0x00 );
        ret.append( MidiParameterType::CC_BS_DELAY, 0x00, 0x00 );
        ret.append( MidiParameterType::CC_BS_LANGUAGE_TYPE, 0x00 );
        return ret;
    }

    /**
     * @brief 歌手変更イベントの NRPN リストを作成する。
     * トラック先頭の歌手変更イベントについては、このメソッドで作成してはいけない。
     * トラック先頭のgenerateNRPN メソッドが担当する
     * @param sequence (Sequence) 出力元のシーケンス
     * @param singerEvent (Event) 出力する歌手変更イベント
     * @param msPreSend (int) ミリ秒単位のプリセンド時間
     * @return (table<NrpnEvent>) NrpnEvent の配列
     */
    static vector<NrpnEvent> generateSingerNRPN( TempoList *tempoList, Event *singerEvent, int preSendMilliseconds ){
        tick_t clock = singerEvent->clock;
        Handle singer_handle;

        double clock_msec = tempoList->getSecFromClock( clock ) * 1000.0;

        double msEnd = tempoList->getSecFromClock( singerEvent->clock + singerEvent->getLength() ) * 1000.0;
        int duration = (int)::floor( ::ceil( msEnd - clock_msec ) );

        int duration0, duration1;
        _getMsbAndLsb( duration, &duration0, &duration1 );

        tick_t actualClock;
        int delay;
        _getActualClockAndDelay( tempoList, clock, preSendMilliseconds, &actualClock, &delay );
        int delayMsb, delayLsb;
        _getMsbAndLsb( delay, &delayMsb, &delayLsb );

        NrpnEvent add( actualClock, MidiParameterType::CC_BS_VERSION_AND_DEVICE, 0x00, 0x00 );
        add.append( MidiParameterType::CC_BS_DELAY, delayMsb, delayLsb, true );
        add.append( MidiParameterType::CC_BS_LANGUAGE_TYPE, singer_handle.language, true );
        add.append( MidiParameterType::PC_VOICE_TYPE, singer_handle.program );

        vector<NrpnEvent> ret;
        ret.push_back( add );
        return ret;
    }

    /**
     * @brief トラックの音符イベントから NRPN のリストを作成する
     * @param sequence (Sequence) 出力元のシーケンス
     * @param track (int) 出力するトラックの番号
     * @param noteEvent (Event) 出力する音符イベント
     * @param msPreSend (int) ミリ秒単位のプリセンド時間
     * @param noteLocation (int) <ul>
     *                               <li>00:前後共に連続した音符がある
     *                               <li>01:後ろにのみ連続した音符がある
     *                               <li>02:前にのみ連続した音符がある
     *                               <li>03:前後どちらにも連続した音符が無い
     *                           </ul>
     * @param lastDelay (int) 直前の音符イベントに指定された、ミリ秒単位のディレイ値。最初の音符イベントの場合は nil を指定する
     * @return (NrpnEvent) NrpnEvent
     * @return (int) この音符に対して設定された、ミリ秒単位のディレイ値
     */
    static NrpnEvent generateNoteNRPN( Track *track, TempoList *tempoList, Event *noteEvent, int msPreSend, int noteLocation, int *lastDelay, int *delay ){
        tick_t clock = noteEvent->clock;
        NrpnEvent add( 0, MidiParameterType::CC_BS_DELAY, 0, 0 );

        tick_t actualClock;
        _getActualClockAndDelay( tempoList, clock, msPreSend, &actualClock, delay );

        //lastDelay と delayがポインタなので注意
        bool addInitialized = false;
        int lastDelayValue;
        if( 0 == lastDelay ){
            add = NrpnEvent(
                actualClock,
                MidiParameterType::CVM_NM_VERSION_AND_DEVICE,
                0x00, 0x00
            );
            lastDelayValue = 0;
            addInitialized = true;
        }else{
            lastDelayValue = *lastDelay;
        }

        if( lastDelayValue != *delay ){
            int delayMsb, delayLsb;
            _getMsbAndLsb( *delay, &delayMsb, &delayLsb );
            if( false == addInitialized ){
                add = NrpnEvent( actualClock, MidiParameterType::CVM_NM_DELAY, delayMsb, delayLsb );
                addInitialized = true;
            }else{
                add.append( MidiParameterType::CVM_NM_DELAY, delayMsb, delayLsb, true );
            }
        }

        if( false == addInitialized ){
            add = NrpnEvent( actualClock, MidiParameterType::CVM_NM_NOTE_NUMBER, noteEvent->note );
        }else{
            add.append( MidiParameterType::CVM_NM_NOTE_NUMBER, noteEvent->note, true );
        }

        // Velocity
        add.append( MidiParameterType::CVM_NM_VELOCITY, noteEvent->dynamics, true );

        // Note Duration
        double msEnd = tempoList->getSecFromClock( clock + noteEvent->getLength() ) * 1000.0;
        double clock_msec = tempoList->getSecFromClock( clock ) * 1000.0;
        int duration = (int)::floor( msEnd - clock_msec );
        int duration0, duration1;
        _getMsbAndLsb( duration, &duration0, &duration1 );
        add.append( MidiParameterType::CVM_NM_NOTE_DURATION, duration0, duration1, true );

        // Note Location
        add.append( MidiParameterType::CVM_NM_NOTE_LOCATION, noteLocation, true );

        if( noteEvent->vibratoHandle.getHandleType() != HandleType::UNKNOWN ){
            add.append( MidiParameterType::CVM_NM_INDEX_OF_VIBRATO_DB, 0x00, 0x00, true );
            string icon_id = noteEvent->vibratoHandle.iconId;
            string num = icon_id.substr( icon_id.length() - 3 );
            int vibrato_type = (int)::floor( StringUtil::parseInt( num, 16 ) );
            int note_length = noteEvent->getLength();
            int vibrato_delay = noteEvent->vibratoDelay;
            int bVibratoDuration = (int)::floor( (note_length - vibrato_delay) / (double)note_length * 127.0 );
            int bVibratoDelay = 0x7f - bVibratoDuration;
            add.append( MidiParameterType::CVM_NM_VIBRATO_CONFIG, vibrato_type, bVibratoDuration, true );
            add.append( MidiParameterType::CVM_NM_VIBRATO_DELAY, bVibratoDelay, true );
        }

        vector<string> spl = noteEvent->lyricHandle.getLyricAt( 0 ).getPhoneticSymbolList();
        ostringstream os;
        for( int j = 0; j < spl.size(); j++ ){
            os << spl[j];
        }
        string s = os.str();
        vector<string> symbols;
        for( int i = 0; i < s.length(); i++ ){
            symbols.push_back( s.substr( i, 1 ) );
        }

        string renderer = track->getCommon()->version;
        if( renderer.substr( 0, 4 ) == string( "DSB2" ) ){
            add.append( (MidiParameterType::MidiParameterTypeEnum)0x5011, 0x01, true );//TODO: (byte)0x5011の意味は解析中
        }

        add.append( MidiParameterType::CVM_NM_PHONETIC_SYMBOL_BYTES, symbols.size(), true );// (byte)0x12(Number of phonetic symbols in bytes)
        int count = -1;
        vector<int> consonantAdjustment = noteEvent->lyricHandle.getLyricAt( 0 ).getConsonantAdjustmentList();
        for( int j = 0; j < spl.size(); j++ ){
            string chars = spl[j];
            for( int k = 0; k < chars.length(); k++ ){
                count = count + 1;
                if( k == 0 ){
                    add.append( (MidiParameterType::MidiParameterTypeEnum)((0x50 << 8) | (0x13 + count)), chars[k], consonantAdjustment[j], true ); // Phonetic symbol j
                }else{
                    add.append( (MidiParameterType::MidiParameterTypeEnum)((0x50 << 8) | (0x13 + count)), chars[k], true ); // Phonetic symbol j
                }
            }
        }
        if( renderer.substr( 0, 4 ) != string( "DSB2" ) ){
            add.append( MidiParameterType::CVM_NM_PHONETIC_SYMBOL_CONTINUATION, 0x7f, true ); // End of phonetic symbols
        }
        if( renderer.substr( 0, 4 ) == string( "DSB3" ) ){
            int v1mean = (int)::floor( noteEvent->pmBendDepth * 60 / 100 );
            if( v1mean < 0 ){
                v1mean = 0;
            }
            if( 60 < v1mean ){
                v1mean = 60;
            }
            int d1mean = (int)::floor( 0.3196 * noteEvent->pmBendLength + 8.0 );
            int d2mean = (int)::floor( 0.92 * noteEvent->pmBendLength + 28.0 );
            add.append( MidiParameterType::CVM_NM_V1MEAN, v1mean, true );// (byte)0x50(v1mean)
            add.append( MidiParameterType::CVM_NM_D1MEAN, d1mean, true );// (byte)0x51(d1mean)
            add.append( MidiParameterType::CVM_NM_D1MEAN_FIRST_NOTE, 0x14, true );// (byte)0x52(d1meanFirstNote)
            add.append( MidiParameterType::CVM_NM_D2MEAN, d2mean, true );// (byte)0x53(d2mean)
            add.append( MidiParameterType::CVM_NM_D4MEAN, noteEvent->d4mean, true );// (byte)0x54(d4mean)
            add.append( MidiParameterType::CVM_NM_PMEAN_ONSET_FIRST_NOTE, noteEvent->pMeanOnsetFirstNote, true ); // 055(pMeanOnsetFirstNote)
            add.append( MidiParameterType::CVM_NM_VMEAN_NOTE_TRNSITION, noteEvent->vMeanNoteTransition, true ); // (byte)0x56(vMeanNoteTransition)
            add.append( MidiParameterType::CVM_NM_PMEAN_ENDING_NOTE, noteEvent->pMeanEndingNote, true );// (byte)0x57(pMeanEndingNote)
            add.append( MidiParameterType::CVM_NM_ADD_PORTAMENTO, noteEvent->pmbPortamentoUse, true );// (byte)0x58(AddScoopToUpInternals&AddPortamentoToDownIntervals)
            int decay = (int)::floor( noteEvent->demDecGainRate / 100.0 * 0x64 );
            add.append( MidiParameterType::CVM_NM_CHANGE_AFTER_PEAK, decay, true );// (byte)0x59(changeAfterPeak)
            int accent = (int)::floor( 0x64 * noteEvent->demAccent / 100.0 );
            add.append( MidiParameterType::CVM_NM_ACCENT, accent, true );// (byte)0x5a(Accent)
        }
        add.append( MidiParameterType::CVM_NM_NOTE_MESSAGE_CONTINUATION, 0x7f, true );// (byte)0x7f(Note message continuation)
        return add;
    }

    /**
     * @brief 指定したシーケンスの指定したトラックから、PitchBend の NRPN リストを作成する
     * @param sequence (Sequence) 出力元のシーケンス
     * @param track (int) 出力するトラックの番号
     * @param msPreSend (int) ミリ秒単位のプリセンド時間
     * @return (table<NrpnEvent>) NrpnEvent の配列
     */
    static vector<NrpnEvent> generatePitchBendNRPN( Track *track, TempoList *tempoList, int msPreSend ){
        vector<NrpnEvent> ret;
        BPList *pit = track->getCurve( "PIT" );
        int count = pit->size();
        int lastDelay = 0;
        for( int i = 0; i < count; i++ ){
            tick_t clock = pit->getKeyClock( i );

            tick_t actualClock;
            int delay;
            _getActualClockAndDelay( tempoList, clock, msPreSend, &actualClock, &delay );
            if( actualClock >= 0 ){
                if( lastDelay != delay ){
                    int delayMsb, delayLsb;
                    _getMsbAndLsb( delay, &delayMsb, &delayLsb );
                    ret.push_back( NrpnEvent( actualClock, MidiParameterType::PB_DELAY, delayMsb, delayLsb ) );
                }
                lastDelay = delay;

                int value = pit->getValue( i ) + 0x2000;
                int msb, lsb;
                _getMsbAndLsb( value, &msb, &lsb );
                ret.push_back( NrpnEvent( actualClock, MidiParameterType::PB_PITCH_BEND, msb, lsb ) );
            }
        }
        return ret;
    }

    /**
     * @brief 指定した時刻における、プリセンド込の時刻と、ディレイを取得する
     * @param tempoList テンポ情報
     * @param clock (int) Tick 単位の時刻
     * @param msPreSend (int) ミリ秒単位のプリセンド時間
     * @return (int) プリセンド分のクロックを引いた Tick 単位の時刻
     * @return (int) ミリ秒単位のプリセンド時間
     */
    static void _getActualClockAndDelay( TempoList *tempoList, tick_t clock, int msPreSend, tick_t *actualClock, int *delay ){
        double clock_msec = tempoList->getSecFromClock( clock ) * 1000.0;

        if( clock_msec - msPreSend <= 0 ){
            *actualClock = 0;
        }else{
            double draft_clock_sec = (clock_msec - msPreSend) / 1000.0;
            *actualClock = (tick_t)::floor( tempoList->getClockFromSec( draft_clock_sec ) );
        }
        *delay = (int)::floor( clock_msec - tempoList->getSecFromClock( (double)*actualClock ) * 1000.0 );
    }

    /**
     * @brief DATA の値を MSB と LSB に分解する
     * @param value (int) 分解する値
     * @param (int) MSB の値
     * @param (int) LSB の値
     */
    static void _getMsbAndLsb( int value, int *msb, int *lsb ){
        if( 0x3fff < value ){
            *msb = 0x7f;
            *lsb = 0x7f;
        }else{
            *msb = 0xff & (value >> 7);
            *lsb = value - (*msb << 7);
        }
    }
};

VSQ_END_NAMESPACE

#endif
