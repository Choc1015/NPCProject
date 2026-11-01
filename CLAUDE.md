# NPCProject

1인칭 NPC 상호작용 게임. Unreal MCP + Blender MCP로 사용자와 같이 만든다.

## 작업 원칙 (사용자 규칙)
- 결정권은 사용자에게 있다. 지금 요청한 기능만 작게 만들고, 거시적인 기획 질문은 하지 않는다.
- 모델/에셋/애니메이션이 필요하면 사용자에게 요청하거나 추천한다. 임시 구체(placeholder) 대신 처음부터 Blender로 만든다.
- 구현은 C++(핵심 로직) + 블루프린트(조정·연결) 혼합.
- 기능 하나가 동작하는 시점마다 velog용 개발일지(`Docs/Devlog/NN_제목.md`, 음슴체 "~했음"에 "~하는 것" 섞기, "내 프롬프트 → AI가 한 것 → 내가 한 것" 형식, 사용자가 설계한 것처럼 쓰지 않기)를 쓰고, 묻지 않고 커밋·푸쉬한다.
- 사용자에게 하는 답변과 마지막 안내 상자는 한국어로 쓴다.

## 언리얼 MCP 작업 규칙

### 역할 분담
- 메인 세션: 시스템 설계, C++/블루프린트 구조 결정, 버그 원인 분석, 서브에이전트 결과 검토
- unreal-worker(Sonnet): 에디터 MCP 조작 전부 — 배치, 수정, 조회, 정리
- 메인 세션은 MCP 도구를 직접 호출하지 않는다. 예외: 디버깅 중 특정 값 1~2개 확인이 필요할 때

### 위임 방법
- 작업은 "무엇을, 어디에, 어떤 값으로"가 분명하게 지시한다. 에셋 경로는 아래 구조의 전체 경로로 준다.
- 서로 독립적인 조작은 한 번의 위임에 묶는다(호출 횟수 줄이기).
- 결과는 요약만 받는다.

### 컨텍스트 절약
- 레벨/액터 전체 덤프 금지. 필요한 범위만 필터링해서 조회.
- 스크린샷은 시각적 확인이 꼭 필요할 때만.
- 이미 아는 구조(아래 "프로젝트 구조")는 MCP로 다시 조회하지 않는다.

### 프로젝트 구조
- 엔진: UE 5.8 런처 빌드 (`C:\Program Files\Epic Games\UE_5.8`). MCP 서버 `unreal` = `http://127.0.0.1:8010/mcp`, 에디터 시작 시 자동 시작(`Config/DefaultEditorPerProjectUserSettings.ini`)
- 빌드: 헤더가 바뀌면 에디터를 닫고 `Engine\Build\BatchFiles\Build.bat NPCProjectEditor Win64 Development -Project="<uproject>" -WaitMutex -NoHotReload`
- 주요 레벨: `/Game/Maps/L_MerchantCamp` (기본 맵). 닫힌 상자형 숲속 야영지 20m×20m(벽 안쪽 x,y=±1000, 천장은 밤하늘 돔 SM_Camp_SkyDome (0,0,550), 별은 M_Camp_Star 발광). 랜턴 기둥 (480,260) + PointLight. 벽은 SM_Camp_ForestWall 8개(스케일 1, 안쪽 면 ±1000, 깊이 바깥 ~1351) + 벽 앞 나무 19그루·고사리 30개(Camp_Forest_*). 풀은 사용자가 칠한 폴리지. PlayerStart (-700,0,100) yaw 0, 상인 (240,60,96) yaw 180, 모닥불 (100,0,0) + PointLight, PostProcess 노출 EV100 0 고정, SkyLight_NightAmbient(DaylightAmbientCubemap, 강도 0.6, 푸른색) 주변광. 메시 액터는 스케일 100(아래 FBX 주의)
  - `/Game/Maps/L_TestField`: 처음 테스트 맵 (Template_Default 복제)
- 게임모드: `/Game/Blueprints/Game/BP_NPCGameMode` (Pawn `BP_PlayerCharacter`, Controller `BP_PlayerController`)
- C++ (`Source/NPCProject/`, 모듈 루트 기준 include)
  - `Interaction/` `IInteractable`, `UInteractionComponent`(Visibility 채널 스피어 트레이스 2.5m)
  - `Dialogue/` `UDialogueData`(노드 ID 기반 대화, 선택지 분기), `UDialogueComponent`(플레이어 쪽 진행, OnDialogueLine/Started/Ended)
  - `NPC/` `ANPCCharacter`(캡슐이 Visibility Block, 대사마다 TalkAnimations 중 하나를 다이내믹 몽타주로 DefaultSlot에 재생)
  - `Player/` `ANPCPlayerCharacter`(1인칭 카메라만), `ANPCPlayerController`(HUD 생성)
  - `UI/` `UGameHUDWidget`(BindWidget: PromptPanel, PromptText, DialoguePanel, SpeakerText, LineText, ChoiceBox, ContinueHint), `UDialogueChoiceButton`
  - `Game/` `ANPCGameMode`
- 블루프린트/에셋
  - `/Game/Blueprints/Player/` BP_PlayerCharacter, BP_PlayerController
  - `/Game/Blueprints/NPC/BP_NPC_Merchant` (DisplayName "떠돌이 상인", Dialogue, SK_MerchantAnime + ABP_MerchantAnime, TalkAnimations 3개, TalkBlendIn/OutTime, `Backpack` StaticMeshComponent — 부착 소켓 칸을 MCP로 못 바꿔서 Construction Script의 AttachComponentToComponent로 BackpackSocket에 붙임, 위치는 소켓에서 조정). 맵에 놓인 인스턴스에도 메시/AnimClass/TalkAnimations를 직접 설정해 둠(CDO 변경이 전파 안 됨)
  - `/Game/Characters/MerchantAnime/` (현재 사용) 트리포 H3.1로 뽑은 애니풍 상인: SK_MerchantAnime(175cm, Mixamo 뼈 65개·손가락 포함), SK_MerchantAnime_Skeleton, ABP_MerchantAnime(Idle 루프 → Slot DefaultSlot), A_MerchantAnime_Idle, A_MerchantAnime_Talk_1~3, M_MerchantAnime_Body, T_MerchantAnime_BaseColor, 배낭 SM_MerchantBackpack(61×35×65cm, 원점=등판 중앙)·M_MerchantBackpack_Body·T_MerchantBackpack_BaseColor. SK_MerchantAnime의 `Spine2` 뼈에 소켓 `BackpackSocket`(사용자가 직접 맞춤). 원본 `SourceArt/Characters/Merchant_Anime/` (Merchant_Anime.blend, _ForMixamo.fbx, Mixamo FBX), 배낭 원본 `SourceArt/Props/Backpack/`. 두 머티리얼 모두 Roughness 1·Specular 0 (무광). 셀 셰이딩 포스트 프로세스는 시도했다가 사용자가 거절해서 삭제함 — 방향은 좀 더 사실적인 쪽
  - `/Game/Characters/Merchant/` (예전 로우폴리, 미사용) SK_Merchant, SK_Merchant_Skeleton, ABP_Merchant, A_Merchant_Idle, A_Merchant_Talk_1~3, M_Merchant_*
  - `/Game/Dialogue/DA_Dialogue_Merchant` (Start → Greet[선택지 3] → Wares/Origin → More[선택지] → Bye). 노드마다 `Voice`(USoundBase) 칸, NPC의 `VoiceAudio` 컴포넌트가 재생
  - `/Game/Audio/Merchant/` VO_Merchant_<NodeId> (ElevenLabs, 원본 `SourceArt/Audio/Merchant/`, 대본 `Docs/Voice/Merchant_VO_Script.md`). 사운드 임포트는 MCP 툴이 없어 사용자가 드래그
  - `/Game/Input/` IA_Move, IA_Look, IA_Jump, IA_Interact, IMC_Default (WASD, Mouse2D Y Negate, Space, E)
  - `/Game/UI/WBP_GameHUD`
  - `/Game/Environment/MerchantCampTripo/` 트리포 소품(스마트 메시, Gemini 레퍼런스, 반실사): SM_CampT_Tent·Campfire·PotTripod·Cart·LogSeat + M_CampT_*(거칠기 0.85, 반사 0)·T_CampT_*_BaseColor, 콜리전 Use Complex As Simple, 스케일 1. 맵의 CampT_* 액터로 예전 Camp_Tent/Campfire/PotTripod/Cart/LogSeat를 대체함(위치는 사용자가 조정). 원본 `SourceArt/Environment/MerchantCampTripo/<Name>/`. 남은 것: 2차(통·상자·자루·장작·그루터기·랜턴 기둥), 3차(바위·나무·덤불)
  - `/Game/Environment/MerchantCamp/` SM_Camp_* (Ground, CliffWall·Canopy(미사용), ForestWall, SkyDome, Tree_A~E, Fern, Rock_A/B, Campfire, PotTripod, Tent, LogSeat, Stump, WoodPile, SackPile, Crate, Barrel, Cart, LanternPost, Bush, MushroomCluster, 폴리지용 GrassClump/GrassTall/Flower) + M_Camp_* 16종, 소품 콜리전 Use Complex As Simple, 풀·버섯은 콜리전 없음
- 폴더 규칙: `/Game/Blueprints/<분류>/`, `/Game/Characters/<캐릭터>/`, `/Game/Dialogue/`, `/Game/Input/`, `/Game/UI/`, `/Game/Maps/`. 원본 아트는 `SourceArt/`, 개발일지는 `Docs/Devlog/`
- 네이밍 규칙: BP_, WBP_, ABP_, SK_, SM_, A_(애니메이션), M_, DA_, IA_, IMC_, L_(레벨)
- 한글 텍스트는 C++에 넣지 않고 BP/데이터 에셋에 넣는다(C++ 기본값은 영어).

## 블렌더 MCP 작업 규칙
- 블렌더 조작은 blender-worker(Sonnet)에게 위임한다. 메인 세션은 블렌더 MCP를 직접 호출하지 않는다.
- 블렌더 → 언리얼 파이프라인: blender-worker가 익스포트 → unreal-worker가 임포트·배치. 메인은 두 결과 요약만 받는다.
- 캐릭터는 blender-worker가 `_ForMixamo.fbx`로 익스포트 → 사용자가 Mixamo에서 리깅·애니메이션(With Skin, FBX Binary, 30fps) → unreal-worker가 기존 스켈레톤으로 임포트.
- 언리얼만 작업하는 세션에서는 블렌더 MCP 서버를 끄고, 블렌더만 작업할 때는 언리얼 MCP 서버를 끈다.

### 에셋 규칙
- 스케일/단위: 블렌더 m 단위로 모델링(사람 키 약 1.8m), FBX 익스포트 시 apply_unit_scale → 언리얼 cm
- 정적 메시 FBX 익스포트 설정(검증됨): `global_scale=1.0, apply_unit_scale=True, apply_scale_options='FBX_SCALE_NONE', axis_forward='X', axis_up='Z', mesh_smooth_type='FACE'` → 언리얼에 cm로 정확히 들어온다. (FBX_SCALE_UNITS는 1/100, global_scale=100까지 주면 ×100.) 처음 13종 SM_Camp_*만 옛 설정이라 액터 스케일 100으로 배치돼 있다
- MCP로 리임포트(덮어쓰기)가 안 되고, 폴리지 타입 생성·폴리지 인스턴스 추가도 안 된다. 리임포트는 삭제 후 임포트, 폴리지는 사용자가 Foliage 모드로 칠한다
- 익스포트 경로: `SourceArt/<분류>/<이름>/` (예: `SourceArt/Characters/Merchant/`), `.blend` 원본도 같은 폴더
- 네이밍 규칙: 메시 `SK_`/`SM_`, 머티리얼 `M_<이름>_<부위>`, Mixamo 결과 `<이름>_Idle.fbx`, `<이름>_Talk_N.fbx`

## 이후 작업 내내 지킬 것
1. 에디터 조작이 필요하면 직접 MCP를 부르지 말고 `unreal-worker`에게 위임한다.
2. 설계·디버깅·코드 작성은 메인에서 한다.
3. 지금 작업과 무관한 MCP 서버가 켜져 있으면 사용자에게 꺼도 되는지 알린다(툴 정의도 토큰을 쓴다).
4. 작업 단위(기능 하나)가 끝나면 사용자에게 `/clear`나 `/compact`를 제안한다.
5. 서브에이전트가 같은 작업에서 2번 이상 실패하면, 그 작업만 메인에서 직접 처리한다.
