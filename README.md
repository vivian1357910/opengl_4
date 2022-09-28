# opengl_4
FLTK-Image Stitching影像拼接

作業簡介：Image Stitching的目的是將多張照片接合成一個全景圖，比如將兩(或多)張相片中重疊的部分，黏接合成一張一張新的相片。


作業環境：Windows10 64bit

執行環境：Visual Studio 2019 x86

編輯器：Visual Studio 2019

語言：C++

套件：FLTK 1.3.8



作業內容：

1.建立一個GUI介面，能夠顯示Load進來的圖片、影像拼接後的結果、Text panel以及可供操作的按鈕

2.按鈕功能 Load、Merge、Reset：

	a.Load：選擇檔案的功能，可以將老師提供的圖片，讀取進使用者介面中，讀取的圖片能夠依序放到介面上
  
	b.Merge：按下拼接功能的按鈕後，將讀取的所有圖片，經過演算法計算，將照片合成一張大圖片
  
	c.Quit：退出程式
  
	d.Reset：重置
  
3.Text panel功能：將讀入圖片、儲存圖片、演算法的過程步驟，都顯示在Text panel的畫面上


reference：

無，未使用opencv內建的函式
