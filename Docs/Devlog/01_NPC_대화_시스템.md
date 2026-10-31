## 이번 목표

- 1인칭으로 돌아다니다가 NPC에게 다가가면 `[E] 대화하기`가 뜨게 한다
- E를 누르면 대화가 시작되고, 중간에 **선택지**로 대화가 갈라지게 한다
- NPC는 임시 구체 말고 처음부터 제대로 된 모델로 간다 → Blender로 직접 만들었다

이번 프로젝트는 Unreal MCP + Blender MCP로 AI(Claude)와 같이 만들고 있다. 결정은 내가 하고, 구현은 C++(핵심 로직) + 블루프린트(조정/연결) 혼합으로 간다.

## 구현한 내용

### 1. Blender로 떠돌이 상인 모델링

로우폴리 스타일로 만들었다. 챙 넓은 모자, 흰 수염, 녹색 튜닉, 붉은 망토, 그리고 상인답게 **침낭 얹은 큰 배낭**에 냄비랑 랜턴까지 달았다. 삼각형 1,700개 정도.

![Blender로 만든 상인](https://media.githubusercontent.com/media/Choc1015/NPCProject/main/Docs/Devlog/images/01_merchant_blender.png)

- Mixamo 자동 리깅을 쓰려고 처음부터 **T포즈**로 만들었다
- 파츠별로 만든 다음 내보낼 때만 하나의 메시로 합쳐서 FBX로 뽑았다
- 손가락이 없는 장갑 형태라 Mixamo에서는 스켈레톤을 **No Fingers**로 골랐다

### 2. Mixamo로 리깅 + 애니메이션

- `Breathing Idle` → With Skin으로 받아서 메시+뼈대+대기 모션을 한 번에 가져왔다
- 대화 모션 3개 → 처음엔 Without Skin으로 받았다가 삽질하고(아래 참고) 결국 전부 **With Skin**으로 다시 받았다
- 임포트할 때 기존 스켈레톤을 지정하면 모션만 쓰고, 같이 딸려 온 메시는 지우면 된다

### 3. C++ 구조

| 클래스 | 역할 |
|---|---|
| `IInteractable` | 상호작용 대상 인터페이스 (`Interact`, `GetInteractionPrompt`, `CanInteract`) |
| `UInteractionComponent` | 카메라 앞을 스피어 트레이스해서 바라보는 대상을 찾는다 (2.5m) |
| `UDialogueData` | 대화 데이터 에셋. 노드 ID로 다음 대사/선택지 분기를 연결한다 |
| `UDialogueComponent` | 플레이어 쪽에서 대화를 진행하고 UI에 대사·선택지를 델리게이트로 뿌린다 |
| `ANPCCharacter` | 말 걸 수 있는 NPC. 말 걸면 플레이어 쪽으로 돌아보고, 대사마다 대화 모션을 하나씩 튼다 |
| `ANPCPlayerCharacter` | 1인칭 플레이어 (카메라만). 대화 중엔 이동/시점 회전을 막는다 |
| `UGameHUDWidget` | 안내 문구 + 대화창. 레이아웃은 WBP에서, 선택지 버튼은 런타임에 만든다 |

대화 데이터는 이렇게 생겼다. 노드마다 ID가 있고, 선택지가 없으면 `NextNodeId`로, 있으면 고른 선택지의 `NextNodeId`로 간다. `None`이면 대화 끝.

```cpp
USTRUCT(BlueprintType)
struct FDialogueNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere) FName NodeId;
	UPROPERTY(EditAnywhere, meta = (MultiLine = true)) FText Text;
	UPROPERTY(EditAnywhere) TArray<FDialogueChoice> Choices;
	UPROPERTY(EditAnywhere) FName NextNodeId;
};
```

인덱스 대신 이름(FName)으로 연결한 이유는, 에디터에서 대사를 중간에 끼워 넣어도 연결이 안 깨지게 하려고.

### 4. 상호작용 흐름

1. `UInteractionComponent`가 0.05초마다 카메라 앞을 트레이스
2. `IInteractable`을 구현한 액터가 걸리면 `OnFocusChanged` → HUD에 `[E] 대화하기`
3. E → NPC의 `Interact()` → 플레이어의 `UDialogueComponent::StartDialogue()`
4. 대사마다 `OnDialogueLine(이름, 대사, 선택지들)` → HUD가 대화창을 그린다
5. 선택지가 있으면 마우스 커서를 보여주고 클릭으로 고른다. 없으면 E로 넘긴다

![상인 앞에서 뜨는 상호작용 안내](https://media.githubusercontent.com/media/Choc1015/NPCProject/main/Docs/Devlog/images/01_interact_prompt.png)

### 5. 대화 모션 섞기 — 루프 → 랜덤 → 몽타주

이 부분은 세 번 갈아엎었다.

1. **대화 모션 하나를 루프** → 같은 손짓만 계속 반복해서 어색했다
2. **모션 3개를 대사마다 랜덤으로 한 번씩** (`PlayAnimation` + 타이머) → 섞이긴 하는데, 모션이 바뀔 때마다 자세가 **뚝뚝 끊겼다**. `PlayAnimation`은 단일 노드 모드라 블렌딩이 없다
3. **애님 블루프린트 + 몽타주** → 최종

최종 구조는 이렇다.

- `ABP_Merchant`: `Idle(루프) → Slot 'DefaultSlot' → Output Pose`
- 대사가 나올 때마다 대화 모션 3개 중 하나(직전 거 제외)를 **다이내믹 몽타주**로 그 슬롯에 재생
- 같은 슬롯에 새 몽타주가 들어오면 이전 몽타주가 자연스럽게 블렌드 아웃되고, 몽타주가 끝나면 아래 깔린 Idle로 블렌드된다

```
idle → talk1 → idle
talk1 → (선택지 고름) → talk2 → idle
```

```cpp
// 직전 인덱스를 피해서 하나 고르기
int32 Index = FMath::RandRange(0, Count - 1);
if (Count > 1 && Index == LastTalkIndex)
{
	Index = (Index + FMath::RandRange(1, Count - 1)) % Count;
}
LastTalkIndex = Index;

// 같은 슬롯의 이전 몽타주는 블렌드 아웃, 끝나면 Idle로 블렌드
AnimInstance->PlaySlotAnimationAsDynamicMontage(Animation, TalkSlotName, TalkBlendInTime, TalkBlendOutTime);
```

몽타주 에셋을 따로 안 만들고 `PlaySlotAnimationAsDynamicMontage`로 시퀀스를 바로 몽타주처럼 틀었다. 블렌드 시간(들어갈 때 0.25초, 빠질 때 0.4초)은 BP에서 조절할 수 있게 열어 뒀다.

주의할 점: `StartDialogue()`가 첫 대사를 **바로** 브로드캐스트하기 때문에, NPC가 델리게이트를 그 뒤에 바인딩하면 첫 대사 모션을 놓친다. 그래서 바인딩을 먼저 하고 대화를 시작하게 순서를 바꿨다.

### 6. 에디터 쪽 (MCP로 작업)

- Enhanced Input: `IA_Move/Look/Jump/Interact` + `IMC_Default` (마우스 Y는 Negate)
- `WBP_GameHUD`: 조준점, 안내 문구, 하단 대화창(이름은 금색, 둥근 테두리)
- `BP_NPC_Merchant`: 이름 "떠돌이 상인", 대화 데이터, `ABP_Merchant`, 대화 모션 3개 연결
- 테스트 맵 `L_TestField`에 상인 배치, 기본 게임모드 지정

## 문제와 해결

### 1. `[E] 대화하기`가 아예 안 뜬다

- **문제**: 상인을 정면으로 봐도 안내 문구가 안 떴다
- **원인**: 두 개가 겹쳤다
  - 상호작용 트레이스는 **Visibility** 채널을 쓰는데, 캐릭터 캡슐의 기본 `Pawn` 프로필은 Visibility를 **Ignore**한다
  - Mixamo에서 가져온 메시엔 **피직스 에셋이 없어서** 메시 쪽에도 충돌체가 없었다
  - 결국 트레이스가 상인한테 한 번도 안 맞고 있었다
- **해결**: NPC 생성자에서 캡슐이 Visibility를 막게 했다

```cpp
// Pawn 프로필은 Visibility를 무시하는데, 플레이어 상호작용 트레이스는 Visibility를 쓴다
GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
```

추가로, 블루프린트 기본값(CDO)을 바꿔도 **이미 레벨에 배치된 인스턴스엔 반영이 안 돼서** 상인을 다시 배치해야 했다.

### 2. 대화 애니메이션 임포트 실패 → 상인이 드러눕다

- **문제 1**: `Talking`을 Without Skin으로 받았더니 임포트가 "본 계층을 못 찾았다"면서 실패했다
- **원인**: 쓰던 임포트 도구가 스켈레탈 메시 임포트 경로라서, 메시 없는 애니메이션 전용 FBX는 못 받는다
- **시도**: Blender에서 Idle FBX(메시+뼈대)에 Talking 액션을 옮겨 붙여서 다시 내보냈다. 임포트는 됐는데…
- **문제 2**: 말을 걸면 상인이 **90도 누워 버렸다**
- **원인**: Mixamo FBX → Blender → FBX로 한 번 왕복하면서 아마추어 축 변환이 원본과 다르게 들어갔다. Mixamo에서 바로 받은 Idle은 멀쩡하고, Blender를 거친 것만 누운 걸로 확인
- **해결**: Blender를 거치지 말고 대화 모션도 **With Skin**으로 받아서 바로 임포트. 교훈: Mixamo 모션은 왕복시키지 말자

### 3. 애님 블루프린트를 붙였더니 상인이 바닥에 구겨졌다

- **문제**: MCP로 애님 블루프린트를 만들어 연결하니 상인이 바닥에 납작하게 구겨졌다
- **원인 1**: MCP로 만든 애님 BP는 **타깃 스켈레톤이 비어 있었고**, 툴로는 나중에 지정할 수도 없었다
- **해결 1**: 스켈레톤 에셋 우클릭 → Create → Anim Blueprint로 직접 만들고, 그래프만 MCP로 짰다
- **원인 2**: 스켈레톤을 넣어도 에디터 뷰포트에선 여전히 구겨져 있었는데, 시뮬레이트로 새로 스폰된 상인은 멀쩡했다. 레벨에 이미 있던 인스턴스가 지워진 예전 애님 BP 상태를 들고 있던 것. 게임에선 문제없다

### 4. 에셋 이름 변경/삭제가 조용히 실패

- **문제**: 임포트한 애니메이션 이름을 바꾸거나 지우려는데 계속 `false`만 돌아왔다
- **원인**: 로그를 보니 `The Editor is currently in a play mode.` — 테스트하고 나서 **PIE가 켜진 채**였다
- **해결**: PIE를 끄니 바로 됐다. 에디터 자동화할 때 PIE 상태부터 확인하자

## 느낀 점: AI는 내가 아는 만큼 잘 만든다

대화 모션을 만들면서 제일 크게 느낀 건 이거다.

처음엔 AI한테 "대화 애니메이션 3개를 랜덤으로 섞어서 넣어 줘"라고만 했다. AI는 요청대로 넣어 주긴 했는데, 모션이 바뀔 때마다 뚝뚝 끊겨서 **전혀 자연스럽지 않았다.** 요청한 기능은 돌아가지만, "자연스럽게"까지 알아서 챙겨 주진 않았다.

그런데 나는 언리얼에 **몽타주**라는 기법이 있다는 걸 알고 있었다. 그래서 "몽타주를 써서 idle과 talk가 자연스럽게 이어지게 해 줘"라고 기준을 잡아서 다시 요청했다. 그랬더니 애님 블루프린트에 슬롯을 만들고 몽타주로 블렌딩하는 구조로 바꿔서, 그제야 자연스럽게 이어졌다.

결국 **AI한테 맡기더라도 내가 어느 정도 지식을 갖고 있어야** 더 좋은 결과가 나온다. 어떤 기법이 있는지, 어떤 방향이 맞는지를 내가 알고 있으면 AI한테 정확한 기준을 줄 수 있고, 그만큼 결과물의 질이 올라간다. AI는 내 지식을 대신하는 게 아니라, 내가 아는 걸 더 빠르게 만들어 주는 도구에 가깝다.

