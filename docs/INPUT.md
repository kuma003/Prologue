# input 形式

シミュレーションを行う際に設定する以下の項目について、ファイルの形式を示します。

- シミュレータの設定 [サンプル](https://github.com/FROM-THE-EARTH/Prologue/blob/main/application/prologue.settings.json)
- マップ設定 [サンプル](https://github.com/FROM-THE-EARTH/Prologue/blob/main/application/input/map/config.json)
- 機体諸元 [単段サンプル](https://github.com/FROM-THE-EARTH/Prologue/blob/main/application/input/json/spec_single.json) [多段サンプル](https://github.com/FROM-THE-EARTH/Prologue/blob/main/application/input/json/spec_multi.json)
- エンジン [サンプル](https://github.com/FROM-THE-EARTH/Prologue/blob/main/application/input/thrust/Sample_G40-4W.txt)
- 風向風速 [サンプル](https://github.com/FROM-THE-EARTH/Prologue/blob/main/application/input/wind/sample.csv)
- 機体速度 vs Cp, Cp_a, Cd_i,Cd_f, Cd_a2, Cna [サンプル](https://github.com/FROM-THE-EARTH/Prologue/blob/main/application/input/aero_coef/sample.csv)

**JSONファイルにおいて以下のようなコメントは実際には許可されていないため、実行時にはコメントを残さないように気をつけてください。**

## シミュレータの設定

シミュレータの設定を行います。<br>
**保存ファイル名**: `prologue.settings.json`<br>
**拡張子**: `json`<br>
**形式**:

```json
{
  "processing": {
    "multi_thread": true,	// マルチスレッドによる処理の高速化。Scatterモードでのみ有効。
    "multi_thread_count": 4	// スレッド数
  },

  "simulation": {
    "dt": 0.0001,                  // 1 ステップで経過する時間。小さければより精度が上がるが計算に時間がかかる。[s]
    // 後方互換性のために残してありますが将来的に削除されます
    // "detect_peak_threshold": 15.0, // 頂点検知によるパラシュート開傘時、最高高度から何メートル落下したら開傘するか[m]

    "scatter": {
      "wind_speed_min": 0.0,    // Scatterモード時、シミュレーション対象の風速の最小値[m/s]
      "wind_speed_max": 7.0,    // Scatterモード時、シミュレーション対象の風速の最大値[m/s]
      "wind_dir_interval": 30.0 // Scatterモード時、風向を何度ずつ変更してシミュレーションするか[deg]
    }
  },

  "result": {
    "precision": 8,          // 結果出力時の数値の小数点以下の桁数
    "step_save_interval": 10 // 何回のステップ毎に結果を保存するか（出力ファイルが巨大になるのを防ぐため）
  },

  "wind_model": {
    "power_constant": 7.0,                        // べき法則の係数
    "power_low_base_alt": 2.0,                    // べき法則の基準高度[m]
    "type": "real",                               // 使用する風モデル　real, original, only_powerlow, no_wind
    "realdata_filename": "wind_data_template.csv" // 風向風速データのファイル名。typeがrealの場合のみ有効。
  },

  "atmosphere": {
    "base_pressure_pascal": 101325, // 高度 0 [m] における気圧[Pa]（正確にはジオポテンシャル高度 0 [m]）
    "base_temperature_celsius": 15  // 高度 0 [m] における気温[°C]（正確にはジオポテンシャル高度 0 [m]）
  }
}
```

## マップ設定

マップの磁気偏角及び緯度経度を設定します。<br>
`izu_land`, `izu_sea`, `nosiro_land`, `izu_sea`のみ設定可能です<br>
**保存フォルダ**: `input/map/config.json`<br>
**拡張子**: `json`<br>
**形式**:

```json
{
  "izu_land": {
    "magnetic_declination": 7.53, // 磁気偏角 [deg]
    "latitude": 34.735972,        // 緯度 [deg]
    "longitude": 139.420944       // 経度 [deg]
  },
  "izu_sea": {
    "magnetic_declination": 7.53,
    "latitude": 34.680197,
    "longitude": 139.43909
  },
  "nosiro_land": {
    "magnetic_declination": 8.9,
    "latitude": 40.138624,
    "longitude": 139.984906
  },
  "nosiro_sea": {
    "magnetic_declination": 8.94,
    "latitude": 40.242865,
    "longitude": 140.01045
  }
}

```

## 機体諸元

機体諸元を記述します。<br>
**保存フォルダ**: `input/spec/`<br>
**拡張子**: `json`<br>
**形式**:

```json
{
  "rocket1": {
    "ref_len": 2.01,  // 機体全長 [m]
    "diam": 0.091,    // 機体直径 [m]
    "mass_i": 8.243,  // 燃焼前機体質量 [kg]
    "mass_f": 7.867,  // 燃焼後機体質量 [kg]
    "CGlen_i": 1.074, // 先端からの燃焼前重心 [kg]
    "CGlen_f": 1.051, // 先端からの燃焼後重心 [kg]
    "Iyz_i": 1.971,   // 燃焼前ピッチヨー慣性モーメント [kg*m^2]
    "Iyz_f": 1.863,   // 燃焼後ピッチヨー慣性モーメント [kg*m^2]
    "Cmq": -3.0,      // ピッチ減衰モーメント係数

    "parachutes": [  // パラシュート諸元
                     // [任意] で指定された条件をandで開傘判定し、満たしたものから順に開きます
      {
        "op_time_from_peak": 0,   // [任意] 頂点到達からの時刻 [s]　
        "op_time_from_launch": 0, // [任意] 離床からの時刻 [s]
        "op_height": 0,           // [任意] 頂点到達後、何メートルしたら開傘するか [m]
        "CdS": 7.0                // パラシュートのCdS
      },
      {
        "op_time_from_peak": 0,
        "op_time_from_launch": 0,
        "op_height": 0,
        "CdS": 2.0 // 複数のパラシュートが開傘する場合、CdSは単純に加算されます
      }
    ],

    // 後方互換性のために残してありますが将来的に削除されます
    // "vel_1st": 10,        // 一段目パラシュートの終端速度 [m/s]
    // "op_type_1st": 0,     // 一段目パラシュートの開傘タイプ　0:頂点検知, 1:時刻 2:頂点検知からの時刻
    // "op_time_1st": 0,     // 一段目パラシュートの開傘時刻 [s]
    // "delay_time_1st": 0,  // [未実装 Issues#14] 一段目パラシュートの開傘遅延 [s]

    "motor_file": "Sample_K240.txt",    // エンジンの推力履歴ファイル名
    "thrust_measured_pressure": 101325, // 推力測定時の気圧 [Pa]　キーが存在しない場合はデフォルト値101325
    "engine_nozzle_diameter": 0,        // ノズル直径 [m]　キーが存在しない場合はデフォルト値0

    "CPlen": 1.38,                  // 先端からの圧力中心 [m] (aero_coef_fileがある場合は無効)
    "Cp_alpha": 0,                  // 先端からの圧力中心傾斜 [m/rad] (aero_coef_fileがある場合は無効)
    "Cd_i": 0.5,                    // 燃焼終了前の抗力係数 (aero_coef_fileがある場合は無効)
    "Cd_f": 0.1,                    // 燃焼終了後の抗力係数 (aero_coef_fileがある場合は無効)
    "Cd_alpha2": 0,                 // 抗力係数傾斜 [/rad^2] (aero_coef_fileがある場合は無効),
    "Cna": 11.747,                  // 法線力整数 (aero_coef_fileがある場合は無効)
    "aero_coef_file": "sample.csv", // 圧力中心(傾斜), 抗力係数(傾斜), 法線力係数 vs 機体速度　のcsvファイル名

    "transitions": [  // 指定した時刻に値を加算 / 減算する。配列で複数指定可能。
      {
        "time": 5.0,  // 時刻 [s]
        "mass": -0.2, // 質量 [kg]
        "Cd": -0.01   // 抗力係数
      },
      {
        "time": 10.0,
        "mass": -0.3,
        "Cd": -0.01
      }
    ]
  },
  
  "rocket2": {},  // 分離後下段。中身はrocket1と同じ。不要な場合は削除する。
  "rocket2": {},  // 分離後上段。中身はrocket1と同じ。不要な場合は削除する。

  "environment": {
    "place": "nosiro_sea",  // マップ名称: "nosiro_sea", "nosiro_land", "izu_sea", "izu_land"
    "rail_len": 5.0,        // ランチャレール長 [m]
    "rail_azi": -60.0,      // ランチャ方位角 [deg]　北から右回り
    "rail_elev": 70         // ランチャ迎角 [deg]
  }
}
```

## エンジン

エンジンの推力を記述します。<br>
**保存フォルダ**: `input/thrust/`<br>
**拡張子**: `txt`<br>
**形式**: 時刻<半角スペース>推力

```
0 10
0.01 20
0.02 30
```

## 風向風速

実際の風向風速データを使用する場合、このファイルを作成し指定します。<br>
**保存フォルダ**: `input/wind/`<br>
**拡張子**: `csv`<br>
**形式**: 1 行目をヘッダとし、2 行目から値を入力する。

| geopotential_height | wind_speed[m/s] | wind_direction[deg](North:0, East:90) |
| ------------------- | --------------- | ------------------------------------- |
| 100                 | 4               | 100                                   |
| 200                 | 6               | 150                                   |
| 500                 | 6               | 180                                   |

## 機体速度 vs Cp, Cp_a, Cd_i, Cd_f, Cd_a2, Cna

機体速度に応じたパラメータを使用する場合、このファイルを作成し指定します。<br>
ファイルが存在しない、または指定されていない場合は機体諸元で指定された値を使用します。<br>
**保存フォルダ**: `input/aero_coef/`<br>
**拡張子**: `csv`<br>
**形式**: 1 行目をヘッダとし、2 行目から値を入力する。

| air_speed[m/s] | Cp_from_nose[m] | Cp_a[m/rad] | Cd_i | Cd_f | Cd_a2[/rad^2] | Cna  |
| -------------- | --------------- | ----------- | ---- | ---- | ------------- | ---- |
| 0              | 1               | 0           | 0.5  | 0.1  | 0             | 11   |
| 20             | 1.1             | 0           | 0.5  | 0.1  | 0             | 11.5 |
| 40             | 1.2             | 0           | 0.5  | 0.1  | 0             | 12   |
