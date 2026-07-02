## 1. Trạng thái Game & Tiến trình chơi (Game States)
Trò chơi bao gồm 3 phân cảnh (Scene) chính tương tác qua lại:
- **Intro Scene (`STATE_INTRO`)**: Màn hình giới thiệu ban đầu. Nhấn phím `Space` hoặc `Enter` để bắt đầu game.
- **World Map Scene (`STATE_WORLD_MAP`)**: Bản đồ lựa chọn màn chơi. 
  - Người chơi sử dụng các phím mũi tên để di chuyển Mario qua lại giữa các nút màn chơi.
  - Các màn chơi được liên kết tuần tự từ **Màn 1 đến Màn 5**. Một màn chơi sẽ bị khóa (không thể đi qua) cho đến khi màn chơi ngay trước nó được hoàn thành.
  - Nhấn `Space` hoặc `Enter` tại nút màn chơi để bắt đầu vượt ải.
- **Playing Scene (`STATE_PLAYING`)**: Phân cảnh chơi chính của từng màn chơi.

---

## 2. Quy tắc vượt ải & Kết quả màn chơi
- **Điều kiện qua màn thường (`ProcessLevelClear`)**: Mario cần vượt qua toàn bộ chướng ngại vật, đi tới cạnh phải ngoài cùng của bản đồ. Khi đó:
  - Điểm thời gian đếm ngược còn thừa trong GameManager sẽ được trừ dần và chuyển đổi thành điểm số tích lũy (tỉ lệ **1 giây = 50 điểm**).
  - Vòng xoay Thẻ bài (**Roulette Card**) ở góc phải HUD sẽ xoay liên tục giữa 4 loại thẻ trong 4 giây, sau đó dừng ngẫu nhiên ở một thẻ và tự động thêm thẻ đó vào kho đồ của người chơi.
  - Sau 6 giây từ khi hoàn thành, màn chơi kết thúc, hệ thống mở khóa màn chơi tiếp theo trên World Map và tự động điều hướng Mario di chuyển tới nút màn chơi đó.
- **Điều kiện thắng cả trò chơi (`ProcessGameWin`)**: Hoàn thành màn cuối cùng (**Level 5**). Điểm thời gian thừa cũng được quy đổi thành điểm số, và sau 6 giây game sẽ tự động đưa người chơi trở lại màn hình Intro.
- **Điều kiện thất bại/Mất mạng (`ProcessMarioDeath`)**: 
  - Xảy ra khi Mario hết mạng (`lives` về 0), bị rơi xuống vực sâu (`y < 0`), hoặc thời gian đếm ngược của màn chơi (bắt đầu từ **300 giây**) chạm mốc 0.
  - Khi chết, nhạc nền tắt, âm thanh `mario_die` vang lên và game đóng băng trong 5 giây trước khi đưa người chơi trở lại World Map.

---

## 3. Các dạng biến hình của Mario (Mario Forms)
Mario có 4 dạng biến hình chính với lượng máu (mạng bảo vệ) và kỹ năng khác nhau:
1. **Small Mario**: Dạng nhỏ bé ban đầu (1 máu). Bị trúng đòn từ quái vật sẽ chết ngay.
2. **Big Mario**: Dạng lớn (2 máu). Có khả năng phá hủy gạch vỡ (`Breakable`) bằng cách nhảy húc đầu từ phía dưới. Bị trúng đòn sẽ co lại thành Small Mario.
3. **Fire Mario**: Dạng bắn lửa (3 máu). Sở hữu khả năng bắn ra quả cầu lửa nảy trên mặt đất bằng phím `Z` (cooldown 0.5s). Bị trúng đòn sẽ lùi về dạng Big Mario.
4. **Scissors Mario (Người kéo)**: Dạng chiến binh kéo đặc biệt (3 máu). Không thể bắn cầu lửa thường, thay vào đó phím `Z` sẽ kích hoạt trạng thái **Phản đòn (Parry)**. Bị trúng đòn sẽ lùi về dạng Big Mario.

---

## 4. Hệ thống Thẻ bài (Card Inventory)
Kho đồ của người chơi hiển thị ở góc phải dưới của HUD, lưu giữ tối đa **3 thẻ bài** khác nhau. Người chơi có thể kích hoạt thẻ bất cứ lúc nào trong màn chơi bằng phím số tương ứng (`1`, `2`, `3`).

### Các loại thẻ bài:
- **Star Card (CARD_STAR - Số 3)**:
  - Cho Mario khả năng bất tử tuyệt đối trong **10 giây**.
  - Nhạc nền tạm dừng và phát bài nhạc bất hủ `star_theme`.
  - Sprite Mario nhấp nháy liên tục 3 màu sắc (Trắng sáng, Đỏ sẫm, Xanh lá nhạt).
  - Tiêu diệt tức khắc bất kỳ quái vật nào chạm vào Mario.
- **Mushroom Card (CARD_MUSHROOM - Số 1)**:
  - Dạng Small: Biến thành Big Mario (máu tăng lên 2).
  - Dạng Fire: Tung chiêu đặc biệt **Fire Blast** (tiêu tốn 1 thẻ).
  - Dạng Scissors: Tung chiêu đặc biệt **World Slash** (tiêu tốn 1 thẻ).
  - Dạng Big thường: Tung chiêu đặc biệt **Rolling Ball** (tiêu tốn 1 thẻ).
- **Flower Card (CARD_FLOWER - Số 2)**:
  - Chưa là Fire Mario: Biến đổi thành Fire Mario (máu tăng lên 3).
  - Đang là Fire Mario: Tung chiêu đặc biệt **Fire Blast** (tiêu tốn 1 thẻ).
- **Scissors Card (CARD_SCISSORS - Số 4)**:
  - Chưa là Scissors Mario: Biến đổi thành Scissors Mario (máu tăng lên 3).
  - Đang là Scissors Mario: Tung chiêu đặc biệt **World Slash** (tiêu tốn 1 thẻ).

> [!NOTE]  
> Nếu bạn nhấn phím dùng thẻ kỹ năng nhưng chiêu thức không thể kích hoạt thành công (ví dụ kẹt trong góc tường chật hẹp, không đủ không gian xuất hiện), game sẽ phát ra âm thanh báo lỗi và **hoàn trả lại thẻ bài** về vị trí cũ trên thanh HUD chứ không làm mất thẻ của bạn.

---

## 5. Chi tiết các Kỹ năng Đặc biệt (Special Skills)

Khi sử dụng thẻ bài trùng với dạng biến hình hiện tại của Mario, trò chơi sẽ tạm dừng (đóng băng) trong 1 giây để thực hiện hiệu ứng niệm chú và tung ra đòn đánh hủy diệt:

### 🌟 Fire Blast (Kỹ năng dạng Lửa)
- **Cơ chế**: Mario tạo ra một luồng lửa khổng lồ có kích thước rộng lớn ($71 \times 23\text{ px}$) bắn thẳng theo chiều ngang hướng mặt của Mario với tốc độ cực nhanh (`0.3f`).
- **Tác dụng**: Thiêu rụi và tiêu diệt mọi kẻ địch trên đường bay của nó, đồng thời phá hủy toàn bộ các khối gạch động (`DynamicBlock`) nó đi qua. Kỹ năng sẽ tự hủy khi bay ra khỏi tầm nhìn của Camera.

### 🌟 Rolling Ball (Kỹ năng dạng Khổng lồ)
- **Cơ chế**: Mario tạo ra một quả bóng năng lượng khổng lồ ($34 \times 34\text{ px}$) lăn dọc trên mặt đất. Quả bóng di chuyển với vận tốc `0.2f` và chịu tác động đầy đủ của trọng lực. Hình ảnh quả bóng tự động xoay tròn mượt mà theo vận tốc thực tế của nó.
- **Tác dụng**: Cán nát và tiêu diệt tất cả kẻ địch nó lăn qua. Khi đập vào các bức tường đứng hoặc khối gạch cứng, quả bóng không biến mất mà sẽ **bội ngược lại chiều đối diện** (`vx = -vx`) và tiếp tục lăn tàn phá màn chơi cho đến khi rơi xuống vực hoặc ra ngoài góc nhìn của camera.

### 🌟 World Slash (Kỹ năng dạng Kéo)
- **Cơ chế**: Nhát chém phá vỡ không gian, tiêu diệt diện rộng toàn màn hình. Khi kích hoạt:
  - Toàn bộ hoạt động vật lý trong game bị đóng băng, nhạc nền tạm ngắt.
  - Một lớp phủ màu đen tối bao trùm lấy màn hình (**Fade Dark** trong 400ms đầu).
  - **Từ 400ms đến 800ms**: 5 nhát chém ánh sáng trắng sắc lẹm với độ dài và góc nghiêng ngẫu nhiên xuất hiện chớp nhoáng lần lượt trên màn hình (mỗi nhát cách nhau 80ms) kèm âm thanh chém kiếm đặc trưng (`slash-sound`).
  - **Tất cả quái vật đang xuất hiện trong tầm nhìn của Camera** sẽ bị tiêu diệt đồng loạt.
  - **Từ 800ms đến 1200ms**: Lớp phủ tối mờ dần trả lại ánh sáng cho màn chơi và tiếp tục game.

---

## 6. Cơ chế Phản đòn Đột phá (Parry System)

Chỉ có thể sử dụng khi Mario đang ở dạng **Scissors Mario**. Bằng cách nhấn phím **Z**, Mario sẽ vào tư thế phòng thủ.

- **Thời gian phản đòn**: **0.25 giây (250ms)**.
- **Hậu quả thất bại**: Nếu không bị kẻ địch hay vật thể nào va chạm trong 250ms này, Mario sẽ thoát trạng thái đỡ đòn và kỹ năng chịu thời gian hồi (**Cooldown**) là **3 giây** (nếu cố nhấn phím `Z` trong lúc cooldown sẽ phát âm thanh báo lỗi).
- **Khi phản đòn thành công (On Parry Success)**:
  - Thời gian hồi (cooldown) được đặt lại về 0 ngay lập tức để có thể phản đòn tiếp.
  - Kẻ địch bị tiêu diệt.

---

## 7. Các sinh vật & Kẻ địch trong thế giới game (Enemies)

- 🍄 **Goomba**: Quái nấm độc nâu cơ bản di chuyển qua lại. Bị tiêu diệt dễ dàng bằng cách dẫm lên đầu (quái bị bẹp dí) hoặc dùng kỹ năng/bắn lửa.
- 🐢 **Koopa Troopa (Rùa)**: Có 3 loại chính:
  - *Green Koopa (Rùa xanh)*: Đi bộ qua lại và tự quay đầu ở mép vực.
  - *Green Flying Koopa (Rùa xanh bay)*: Nhảy lò cò liên tục theo quỹ đạo parabol.
  - *Red Koopa (Rùa đỏ)*: Tương tự rùa xanh (nhưng nó màu đỏ).
  - *Cơ chế mai rùa*: Khi bị dẫm lên đầu, chúng sẽ rút đầu vào mai rùa (`SHELL`). Lúc này Mario có thể đến gần để đá mai rùa bay đi (`SHELL_SPINNING`), trượt đi và tiêu diệt các quái vật khác. Nếu để yên mai rùa trong 5 giây, mai rùa sẽ rung lắc dữ dội (`SHELL_SHAKING`) trong 3 giây tiếp theo trước khi Koopa chui ra hoạt động bình thường.
- 🔨 **Hammer Bro**: Rùa chiến binh ném búa di chuyển qua lại quanh vị trí gác, ném các cây búa bay theo hình vòng cung liên tục và có khả năng tự nhảy lên các tầng gạch cao hơn hoặc nhảy lọt xuống dưới.
- 🌵 **Piranha Plant & Venus Fire Trap**: Hoa ăn thịt người ẩn nấp trong các ống nước xanh lá:
  - *Piranha Plant*: Tự động trồi lên cắn người chơi và thụt xuống theo chu kỳ.
  - *Venus Fire Trap*: Có thể xoay đầu hướng về phía Mario (4 góc: Trên-Trái, Dưới-Trái, Trên-Phải, Dưới-Phải), phun ra cầu lửa tấn công từ xa.
  - *Mẹo*: Nếu Mario đứng quá gần mép ống nước, hoa sẽ không thể trồi lên.

---

## 8. Các khối gạch & Chướng ngại vật (Blocks)
- **Ground / StaticBlock**: Khối đất tĩnh tạo khung nền địa hình vững chắc cho màn chơi.
- **Breakable (Gạch vỡ)**: Các viên gạch nâu có thể vỡ vụn khi bị Mario dạng lớn húc đầu từ dưới lên, bị kỹ năng bắn trúng hoặc mai rùa đập vào. Phá hủy gạch mang lại 10 điểm và có hiệu ứng mảnh vỡ văng tung tóe. Chúng có hệ thống dựng hình tự động thay đổi kết cấu đồ họa rìa cỏ theo các ô lân cận (Isolated, Left, Right, Center).
- **LuckyBlock (Hộp câu hỏi vàng)**: Hộp chứa phần thưởng. Húc từ phía dưới sẽ nhả ra 1 vật phẩm ngẫu nhiên trong 4 loại (`Mushroom`, `Flower`, `Star`, `Scissors`) dưới dạng hạt mầm nảy lên từ từ (`Sprouting`), cộng 100 điểm cho người chơi và vô hiệu hóa.

---

## 9. Phím tắt điều khiển & Tính năng Nhà phát triển (Controls & Cheats)

### Phím điều khiển thông thường:
- **Mũi tên Trái / Phải**: Di chuyển chạy qua lại.
- **Mũi tên Xuống**: Ngồi xuống / Chui xuống ống nước.
- **Phím Space (Cạch)**: Nhảy. (Giữ phím để nhảy cao hơn, nhả sớm để ngắt lực nhảy thấp).
- **Phím Z**: Bắn lửa (Dạng Lửa) / Phản đòn (Dạng Kéo).
- **Phím số 1, 2, 3**: Sử dụng thẻ bài tại ô slot tương ứng trên HUD.

### Phím tắt dành cho Nhà phát triển (Developer Cheat Keys):
- **Phím F1**: Bỏ qua màn chơi hiện tại lập tức (Trực tiếp clear màn thường, nếu ở Màn 5 sẽ thắng cả game).
- **Phím F2**: Bật/tắt hiển thị khung va chạm hình chữ nhật màu đỏ (**Bounding Box**) của tất cả đối tượng vật lý trong game để kiểm tra lỗi va chạm.
- **Phím F3**: Kích hoạt **Debug Mode ở World Map** giúp bỏ qua điều kiện khóa màn chơi, cho phép Mario di chuyển tự do đến bất kỳ Level nào mong muốn.
- **Phím F4**: Kích hoạt **Chế độ Debug Thẻ bài**. Khi bật chế độ này, người chơi có thể tự do tạo thêm thẻ bài vào kho đồ bằng các phím:
  - Phím `A`: Nhận thẻ Nấm (Mushroom Card).
  - Phím `S`: Nhận thẻ Hoa (Flower Card).
  - Phím `D`: Nhận thẻ Sao (Star Card).
  - Phím `F`: Nhận thẻ Kéo (Scissors Card).
- **Phím F5**: Tạm dừng (Pause) toàn bộ trò chơi và nhạc nền. Nhấn lại để tiếp tục chơi.
- **Phím R**: Tự gây sát thương lên Mario (giảm dạng biến hình hoặc tự sát) để test cơ chế hồi sinh và bất tử tạm thời.
