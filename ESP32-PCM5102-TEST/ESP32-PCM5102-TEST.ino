#include "ESP32WaveOut.h"

#define SAMPLE_RATE 48000

WaveOut hWOut(SAMPLE_RATE);  // 音频输出实例

// 双声道信号类型
struct StereoSignal {
  int32_t l; // 左声道
  int32_t r; // 右声道
};

const int BufLen = hWOut.GetBufferLen();  // 缓冲区长度
int32_t *WavBuf = (int32_t *)malloc(BufLen * sizeof(int32_t));  // 分配缓冲区内存

int32_t t1 = 0;  // 时间变量

// 产生带有合唱效果的双声道锯齿波
StereoSignal Saw1_Stereo(int SawFreq, int UniN, float delta) {
  // 更新时间变量
  t1 += SawFreq / (float)SAMPLE_RATE;

  float tmp1 = 1.0f;  // 初始化幅度
  int32_t tmp2_l = 0, tmp2_r = 0;  // 临时累积变量
  bool tmp_switch = true;  // 用于切换左右声道

  // 对每一个合唱声部进行处理
  for (int i = 0; i < UniN; ++i) {
    // 如果是左声道
    if (tmp_switch) {
      tmp2_l += (int32_t)((tmp1 *= delta) * t1) % SAMPLE_RATE;
    } else {
      tmp2_r += (int32_t)((tmp1 *= delta) * t1) % SAMPLE_RATE;
    }
    tmp_switch = !tmp_switch;  // 切换到另一个声道
  }

  // 计算平均值和差异值
  int32_t avg = (tmp2_l + tmp2_r) / UniN;
  int32_t diff = (tmp2_l - tmp2_r) / UniN;

  // 返回双声道信号
  return (StereoSignal){avg - (SAMPLE_RATE >> 1), diff - (SAMPLE_RATE >> 1)};
}

void setup() {
  Serial.begin(115200);  // 初始化串口通信
  memset(WavBuf, 0, BufLen * sizeof(int32_t));  // 清空缓冲区
  hWOut.Start();  // 启动音频输出
}

void loop() {
  float datL = 0, datR = 0;
  StereoSignal sign = Saw1_Stereo(440, 4, 0.99);  // 生成锯齿波信号

  // 填充缓冲区
  for (int i = 0; i < BufLen; i += 2) {
    datL += sign.l;
    datR += sign.r;
    WavBuf[i + 0] = datL * 2500.0;  // 填入左声道数据
    WavBuf[i + 1] = datR * 2500.0;  // 填入右声道数据
  }

  uint64_t PlayTime = hWOut.PlayAudio(WavBuf);  // 播放缓冲区内容
}