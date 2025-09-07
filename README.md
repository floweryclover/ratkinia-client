# Ratkinia Client
* Unreal Engine 5로 구현된 Ratkinia 프로젝트의 클라이언트
* SPSC 송수신 링버퍼로 게임 스레드와 네트워크 스레드간 효율적인 동기화
* CRTP 클래스 형태로 자동 생성되는 RPC 함수
    * STC 수신 RPC는 게임모드가 해당 클래스를 상속하여 처리하고 싶은 함수 정의
    * CTS 송신 RPC는 URatkiniaClientSubsystem이 상속하고 있으므로 해당 클래스의 RPC 함수 바로 호출

### CTS RPC 호출 로직
![CTS](https://github.com/user-attachments/assets/893b8fff-04ed-428f-8108-0e29a05ca5ab)

### STC RPC 처리 로직
![STC_1](https://github.com/user-attachments/assets/1bfa2a50-4c05-4d63-b23f-eb9dbbc87389)

### STC RPC 정의를 위한 자동 생성 CRTP 상속
![STC_2](https://github.com/user-attachments/assets/0e85c0ae-3a59-452a-94bc-2fdee3c225cb)

### 회원가입 요청 후 결과 화면
![Register](https://github.com/user-attachments/assets/7779be26-896e-4fbc-bff3-ad740b46f5cd)

### 캐릭터 조회 후 선택 가능 화면
![SelectCharacter](https://github.com/user-attachments/assets/09b1da12-5dff-45db-b723-db3c6ae20ea8)