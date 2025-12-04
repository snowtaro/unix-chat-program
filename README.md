# POSIX 기반 터미널 채팅 프로그램

## 프로젝트 소개

이 프로젝트는 POSIX 공유 메모리와 세마포어, ncurses 기반 UI, 그리고 멀티프로세스 구조를 사용한 터미널 채팅 프로그램의 클라이언트 구현입니다.


## 주요 기능

* 실시간 터미널 기반 채팅 (멀티 클라이언트 지원)
* 사용자 목록 실시간 표시
* 귓속말 기능: `/whisper <nickname> <message>`
* UTF-8 입력 및 출력 (와이드 문자 지원)
* 사용자별 색상 지정
* POSIX 공유 메모리 및 세마포어로 IPC 구현
* ncurses로 구성된 세 창: 채팅창, 입력창, 사용자 목록창


## 디렉토리 구조

```
unix-chat-program/
├── include/
│   └── chat_common.h
│
├── src/
│   ├── chat_client.c
│   └── chat_server.c
│
├── Makefile         
└── README.md
```


## 설치 및 실행 방법

### 의존성

* ⭐ Linux 또는 POSIX 시스템
* gcc
* libncursesw (와이드 문자 지원)

   ```
   # libncursesw가 없을때
   sudo apt update
   sudo apt install libncursesw5-dev libncurses5-dev
   ```

### 빌드

```
# make
make
```

### 실행

```
# 서버 실행 (백그라운드)
./bin/server -d &

# 클라이언트 실행:
./bin/client

# 닉네임 입력 → 메시지 입력
```


## 사용 방법

* 일반 채팅: 입력 후 Enter
* 귓속말:
  /whisper <닉네임> <메시지>
* 접속자 목록: 우측 창에서 실시간 표시
* 종료: Ctrl+C


## 개선 가능성

* 채팅 내역 스크롤 및 검색 기능
* 명령어 자동완성 및 도움말
* 사용자 상태 표시 (접속/비접속)
* 소켓 기반 IPC로 확장