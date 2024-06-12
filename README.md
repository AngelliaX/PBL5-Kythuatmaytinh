<h1 align="left">PBL5 - Dự án Kỹ thuật máy tính</h1>

###

<p align="left">🚚 Nhóm 1 - Đếm số lượng ống trên xe hàng.<br> Repo này chứa chương trình của phần cứng, và server AI</p>

###

<h2 align="left">Giải thích nội dung</h2>
<p align="left">
  PBL5 - AI: Chứa code của AI Server.<br>
  PBL5-Arduino: Chứa code của Arduino.<br>
  PBL5-Esp32cam: Chứa code của Esp32-cam.<br>
  labeled_data: Chứa dữ liệu huấn luyện 2 đợt của AI. 
</p>

###

###

<h2 align="left">Cách chạy AI server</h2>

<p align="left">
  Bước 1: Tải thư mục <code>weights</code> chứa model đã huấn luyện tại <a href="https://drive.google.com/drive/folders/1-oAPeadHxmlw1xbCvZdOYPtxVPoZ65uC?usp=sharing">Google Drive</a> và đặt vào <code>C:\PBL5 - AI\runs\detect\train3\</code><br>
  Bước 2: Tải các thư viện cần thiết: <code>pip install -r requirements.txt</code> <br>
  Bước 2: Di chuyển tới thư mục chứa chương trình <code>cd C:\PBL5 - AI\flask</code><br>
  Bước 3: Chạy chương trình <code>py server.py</code>. Kết thúc.<br>
</p>

![Model Diagram](https://i.imgur.com/WXH18eB.png)

###

<h2 align="left">Cách chạy Arduino</h2>
<p align="left">
  Bước 1: Nạp code vào arduino. Kết thúc.<br>
</p>

###

<h2 align="left">Cách chạy Esp32-cam</h2>
<p align="left">
  Bước 1: Cài lại tên và mật khẩu Wifi tại WifiCam.ino<br>
  Bước 2: Cài lại IP tương ứng của Flask và SpringBoot tại handlers.cpp<br>
  Bước 3: Nạp code vào Esp32-cam. Kết thúc.
</p>

###