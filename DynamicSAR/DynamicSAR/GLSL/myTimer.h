#ifndef MYTIMER_H
#define MYTIMER_H

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>


/**
 * @brief   時間計測するクラス
 */
class MyTimer
{
private:
	LARGE_INTEGER Freq;			// 周波数
	LARGE_INTEGER lastCount;	// 前回のカウント
	LARGE_INTEGER thisCount;	// 最新のカウント

	int frameCount;				// フレームのカウント
	double updateTime;			// fps更新の間隔(秒)
	double fixedFps;			// 固定したいfpsの値
	double fps;					// fps

public:
	MyTimer() 
		: frameCount (0)
		, updateTime (1.0)
		, fixedFps (60.0)
		, fps (0.0)
	{
		// 周波数の取得
		QueryPerformanceFrequency( &Freq );
		start();	// 初期測定
	}
	~MyTimer() {}

	// 計測開始
	inline void start()
	{
		QueryPerformanceCounter( &lastCount );		// カウントの保存
	}

	// 計測終了
	inline void stop()
	{
		QueryPerformanceCounter( &thisCount );		// カウントの保存
	}

	// 秒の計測
	inline double Sec()
	{
		return (double)( thisCount.QuadPart - lastCount.QuadPart ) / Freq.QuadPart;
	}

	// ミリ秒の計測
	inline double MSec()
	{
		return 1000.0 * (double)( thisCount.QuadPart - lastCount.QuadPart ) / Freq.QuadPart;
	}

	// 秒の表示
	inline void printSec()
	{
		std::cout << Sec() << "[sec]" << std::endl;
	}

	// ミリ秒の表示
	inline void printMSec()
	{
		std::cout << MSec() << "[msec]" << std::endl;
	}

	// FPSの取得
	inline double getFps() {	return fps; }

	// FPS更新間隔のセット
	inline void setUpdateFps(double t)
	{
		updateTime = t;
	}

	// 固定したいFPSの値のセット
	inline void setFixedFps(double f) 
	{
		fixedFps = f;
	}


	// FPS計測の更新
	inline void fpsUpdate()
	{
		stop();

		// 1フレーム目ならば時刻を記録
		if ( frameCount == 0 ) 
		{
			start();	
		}
		// fpsの更新時間がきたら更新
		else if ( Sec() >= updateTime )
		{
			fps = 1.0 / (Sec() / (double)frameCount);
			frameCount = 0;
			start();
		}

		frameCount++;
	}

	// FPSの固定
	inline void wait()
	{
		LARGE_INTEGER nowCount;
		QueryPerformanceCounter( &nowCount );
		double tookTime = 1000.0 * (double)( nowCount.QuadPart - lastCount.QuadPart ) / Freq.QuadPart;	// かかった時間
		double waitTime = (double)frameCount * 1000.0 / fixedFps - tookTime;					// 待つべき時間
		if(waitTime > 0.0)
			Sleep((DWORD)waitTime);	// 待機
	}

	// FPSの表示
	inline void printFps(bool wait = true)
	{
		if( wait && frameCount != 1)
		{
			return;
		}
		std::cout << fps << "[fps]" << std::endl;
	}
};


#endif