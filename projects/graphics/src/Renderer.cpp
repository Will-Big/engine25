#include "pch.h"
#include "Renderer.h"

#include <d3dcompiler.h>

using namespace Microsoft::WRL;

namespace graphics
{
	// 실제 구현 클래스
	class RendererImpl final : public IRenderer
	{
	public:
		RendererImpl() = default;
		~RendererImpl() override;

		void Initialize(HWND hWnd, UINT width, UINT height) override;

		void Render() override;

	private:
		void createDeviceAndFactory();
		void createCommandObjects();
		void createSwapChain(HWND hWnd, UINT width, UINT height);
		void createDescriptorHeaps();
		void createRenderTargetViews();
		void createDepthStencilBuffer(UINT width, UINT height);
		void createRootSignature();
		void createPipelineState();
		void populateCommandList() const;
		void waitForPreviousFrame();

		// DirectX12 관련 멤버 변수
		ComPtr<ID3D12Device>                _device;
		ComPtr<IDXGIFactory4>               _factory;
		ComPtr<ID3D12CommandQueue>          _commandQueue;
		ComPtr<ID3D12CommandAllocator>      _commandAllocator;
		ComPtr<ID3D12GraphicsCommandList>   _commandList;
		ComPtr<IDXGISwapChain3>             _swapChain;
		UINT                                _frameIndex = 0;

		ComPtr<ID3D12DescriptorHeap>        _rtvHeap;
		ComPtr<ID3D12Resource>              _renderTargets[2];
		UINT                                _rtvDescriptorSize = 0;

		ComPtr<ID3D12Resource>              _depthStencilBuffer;
		ComPtr<ID3D12DescriptorHeap>        _dsvHeap;
		UINT                                _dsvDescriptorSize = 0;

		ComPtr<ID3D12RootSignature>         _rootSignature;
		ComPtr<ID3D12PipelineState>         _pipelineState;

		D3D12_VIEWPORT                      _viewport = {};
		D3D12_RECT                          _scissorRect = {};
		float                               _clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };

		D3D12_VERTEX_BUFFER_VIEW            _vertexBufferView = {};
		D3D12_INDEX_BUFFER_VIEW             _indexBufferView = {};
		UINT                                _indexCount = 0;

		ComPtr<ID3D12Fence>                 _fence;
		UINT64                              _fenceValue = 0;
		HANDLE                              _fenceEvent = nullptr;

		ComPtr<ID3D12DescriptorHeap>		_cbvSrvUavHeap;
		CD3DX12_GPU_DESCRIPTOR_HANDLE		_gpuHandleStart = {};
	};

	RendererImpl::~RendererImpl()
	{
		// GPU가 남은 작업을 완료할 때까지 대기
		waitForPreviousFrame();

		// 이벤트 핸들 닫기
		if (_fenceEvent)
		{
			CloseHandle(_fenceEvent);
			_fenceEvent = nullptr;
		}

#if defined(_DEBUG)
		ComPtr<ID3D12DebugDevice> debugDev;
		if (SUCCEEDED(_device.As(&debugDev)))
			debugDev->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);

#endif
	}

	void RendererImpl::Initialize(HWND hWnd, UINT width, UINT height)
	{
		createDeviceAndFactory();
		createCommandObjects();
		createSwapChain(hWnd, width, height);
		createDescriptorHeaps();
		createRenderTargetViews();
		createDepthStencilBuffer(width, height);
		createRootSignature();
		createPipelineState();
	}

	void RendererImpl::Render()
	{
		// 커맨드 리스트에 이번 프레임 렌더링 명령 기록
		populateCommandList();

		// 기록된 커맨드 리스트 GPU 제출
		ID3D12CommandList* lists[] = { _commandList.Get() };
		_commandQueue->ExecuteCommandLists(_countof(lists), lists);

		// 화면에 출력
		THROW_IF_FAILED(_swapChain->Present(1, 0));

		// 이전 프레임이 완전히 끝날 때까지 대기하고 인덱스 갱신
		waitForPreviousFrame();
	}

	// extern "C" 팩토리 구현
	extern "C" GRAPHICS_API IRenderer* CreateRenderer()
	{
		return new RendererImpl();
	}

	extern "C" GRAPHICS_API void DestroyRenderer(IRenderer* pRenderer)
	{
		delete pRenderer;
	}

	void RendererImpl::createDeviceAndFactory()
	{
		// DirectX12 디버그 레이어 활성화 (디버그 빌드에서만)
#if defined(_DEBUG)
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
			debugController->EnableDebugLayer();
#endif

		// 1) DXGI Factory 생성
		THROW_IF_FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&_factory)));

		// 2) D3D12 Device 생성
		// nullptr → 기본 어댑터, D3D_FEATURE_LEVEL_11_0 이상 요구
		THROW_IF_FAILED(D3D12CreateDevice(
			nullptr,
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&_device)
		));
	}

	void RendererImpl::createCommandObjects()
	{
		// 1) Command Queue 생성: GPU가 명령들을 순차적으로 실행할 큐
		D3D12_COMMAND_QUEUE_DESC cqDesc{};
		cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;  // 그래픽·컴퓨트 모두 허용
		cqDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		THROW_IF_FAILED(_device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&_commandQueue)));

		// 2) Command Allocator 생성: Command List가 기록할 메모리 풀
		THROW_IF_FAILED(_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(&_commandAllocator)
		));

		// 3) Graphics Command List 생성: 실제 명령을 기록하는 객체
		THROW_IF_FAILED(_device->CreateCommandList(
			0,                                // 노멀 커맨드 리스트 플래그
			D3D12_COMMAND_LIST_TYPE_DIRECT,   // Direct list
			_commandAllocator.Get(),          // 연결할 Allocator
			nullptr,                          // 초기 PSO(nullptr=없음)
			IID_PPV_ARGS(&_commandList)
		));

		// CreateCommandList 호출 직후에는 열려(open) 있는 상태
		THROW_IF_FAILED(_commandList->Close());

		// 4) Command Queue 생성 후...
		THROW_IF_FAILED(_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)));
		_fenceValue = 1;

		// Win32 이벤트 생성
		_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (_fenceEvent == nullptr)
			throw std::runtime_error("FenceEvent 생성 실패");
	}

	void RendererImpl::createSwapChain(HWND hWnd, UINT width, UINT height)
	{
		// 1) 스왑 체인 설명자 설정
		DXGI_SWAP_CHAIN_DESC1 scDesc{};
		scDesc.BufferCount = 2;                                // 더블 버퍼
		scDesc.Width = width;
		scDesc.Height = height;
		scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;            // 8비트 RGBA
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // 렌더 타겟으로 사용
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;     // 최신 권장 플리핑 모드
		scDesc.SampleDesc.Count = 1;                           // 멀티샘플링 없이

		// 2) IDXGISwapChain1 생성
		ComPtr<IDXGISwapChain1> swap1;
		THROW_IF_FAILED(_factory->CreateSwapChainForHwnd(
			_commandQueue.Get(),   // Submit할 커맨드 큐
			hWnd,                  // 렌더링 대상 윈도우 핸들
			&scDesc,
			nullptr, nullptr,
			&swap1
		));

		// 3) IDXGISwapChain3로 캐스팅
		THROW_IF_FAILED(swap1.As(&_swapChain));

		// 4) 현재 백 버퍼 인덱스 저장
		_frameIndex = _swapChain->GetCurrentBackBufferIndex();
	}

	void RendererImpl::createDescriptorHeaps()
	{
		HRESULT hr;

		// 1) RTV 힙 생성
		{
			D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
			rtvHeapDesc.NumDescriptors = 2;
			rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			THROW_IF_FAILED(_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&_rtvHeap)));
			_rtvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		}

		// 2) DSV 힙 생성
		{
			D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
			dsvHeapDesc.NumDescriptors = 1;
			dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			THROW_IF_FAILED(_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&_dsvHeap)));
			_dsvDescriptorSize = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		}

		// 3) CBV/SRV/UAV 힙 생성
		{
			D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
			cbvSrvUavHeapDesc.NumDescriptors = 1; // 필요한 디스크립터 개수만큼
			cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			THROW_IF_FAILED(_device->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(&_cbvSrvUavHeap)));

			// GPU에서 셰이더가 참조할 수 있도록 시작 핸들 저장
			_gpuHandleStart = _cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart();
		}
	}

	void RendererImpl::createRenderTargetViews()
	{
		// RTV 힙의 시작 핸들
		auto rtvHandle = _rtvHeap->GetCPUDescriptorHandleForHeapStart();

		// 스왑체인의 각 백버퍼를 가져와서 RTV로 연결
		for (UINT i = 0; i < 2; ++i)
		{
			// 1) 백버퍼 리소스 가져오기
			THROW_IF_FAILED(_swapChain->GetBuffer(
				i, IID_PPV_ARGS(&_renderTargets[i])
			));

			// 2) RTV 생성
			_device->CreateRenderTargetView(
				_renderTargets[i].Get(),
				nullptr,     // 기본 RTV 디스크립터 사용
				rtvHandle
			);

			// 3) 다음 백버퍼를 위해 핸들 오프셋
			rtvHandle.ptr += _rtvDescriptorSize;
		}
	}

	void RendererImpl::createDepthStencilBuffer(UINT width, UINT height)
	{
		// 1) DSV 힙 생성
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&_dsvHeap));

		// 핸들 오프셋
		_dsvDescriptorSize = _device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV
		);

		// 2) 깊이-스텐실 리소스 설명
		D3D12_RESOURCE_DESC dsDesc = CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_D32_FLOAT,   // 32비트 깊이
			width, height, 1, 0,     // 크기, 배열 1, Mip 0
			1, 0,                    // 샘플 1, 품질 0
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
		);

		// 3) 클리어용 값
		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;


		// 4) 리소스 할당
		const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		THROW_IF_FAILED(_device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&dsDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,  // 초기 상태: 깊이 쓰기
			&clearValue,
			IID_PPV_ARGS(&_depthStencilBuffer)
		));

		// 5) DSV 생성
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;

		_device->CreateDepthStencilView(
			_depthStencilBuffer.Get(),
			&dsvDesc,
			_dsvHeap->GetCPUDescriptorHandleForHeapStart()
		);
	}

	void RendererImpl::createRootSignature()
	{
		// 1) 루트 시그니처 설명자: IA(Input Assembler) 입력 레이아웃만 허용
		CD3DX12_ROOT_SIGNATURE_DESC rsDesc{};
		rsDesc.Init(
			0,               // 루트 파라미터 수
			nullptr,         // 루트 파라미터 배열
			0,               // 정적 샘플러 수
			nullptr,         // 정적 샘플러 배열
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		// 2) 직렬화 + Blob 생성
		ComPtr<ID3DBlob> sigBlob;
		ComPtr<ID3DBlob> errorBlob;
		HRESULT hr = D3D12SerializeRootSignature(
			&rsDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&sigBlob,
			&errorBlob
		);
		if (FAILED(hr))
		{
			auto msg = static_cast<const char*>(errorBlob->GetBufferPointer());
			throw std::runtime_error(std::string("RootSig Serialize Failed: ") + msg);
		}

		// 3) 루트 시그니처 생성
		THROW_IF_FAILED(_device->CreateRootSignature(
			0,
			sigBlob->GetBufferPointer(),
			sigBlob->GetBufferSize(),
			IID_PPV_ARGS(&_rootSignature)
		));
	}



	void RendererImpl::createPipelineState()
	{
		// 1) 루트 시그니처가 준비되어 있어야 합니다.
		//    CreateRootSignature()가 먼저 호출돼야 함.

		// 2) 셰이더 Blob 로드
		ComPtr<ID3DBlob> vsBlob, psBlob;
		THROW_IF_FAILED(D3DReadFileToBlob(L"vertexShader.cso", &vsBlob));
		THROW_IF_FAILED(D3DReadFileToBlob(L"pixelShader.cso", &psBlob));

		// 3) 입력 레이아웃 정의
		D3D12_INPUT_ELEMENT_DESC inputLayout[] = 
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		// 4) PSO 설명자 채우기
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
		psoDesc.pRootSignature = _rootSignature.Get();
		psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
		psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;

		// 5) PSO 생성
		THROW_IF_FAILED(_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&_pipelineState)));
	}

	void RendererImpl::populateCommandList() const
	{
		// 1) Command Allocator reset
		THROW_IF_FAILED(_commandAllocator->Reset());

		// 2) Command List reset (초기 PSO 바인딩)
		THROW_IF_FAILED(_commandList->Reset(_commandAllocator.Get(), _pipelineState.Get()));

		// 3) 뷰포트·시저 설정
		// → 클래스에 D3D12_VIEWPORT _viewport;, D3D12_RECT _scissorRect; 가 필요합니다.
		_commandList->RSSetViewports(1, &_viewport);
		_commandList->RSSetScissorRects(1, &_scissorRect);

		// 4) 백버퍼 상태 → 렌더타깃 상태 전환
		{
			const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				_renderTargets[_frameIndex].Get(),
				D3D12_RESOURCE_STATE_PRESENT,
				D3D12_RESOURCE_STATE_RENDER_TARGET);
			_commandList->ResourceBarrier(1, &barrier);
		}

		// 5) RTV 핸들 계산
		const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
			_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
			static_cast<INT>(_frameIndex),
			_rtvDescriptorSize);

		// 6) DSV 핸들 계산
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(
			_dsvHeap->GetCPUDescriptorHandleForHeapStart());

		// 7) RTV + DSV 바인딩 (깊이 스텐실을 사용하므로 둘 다 넘겨야 함)
		_commandList->OMSetRenderTargets(
			1,               // RTV 개수
			&rtvHandle,      // RTV 핸들 배열
			FALSE,
			&dsvHandle       // DSV 핸들 포인터
		);

		// 8) 클리어 (RTV & DSV)
		_commandList->ClearRenderTargetView(rtvHandle, _clearColor, 0, nullptr);
		_commandList->ClearDepthStencilView(
			dsvHandle,
			D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
			1.0f, 0,
			0, nullptr);


		// 9) 루트 시그니처 바인딩
		_commandList->SetGraphicsRootSignature(_rootSignature.Get());

		// 10) 셰이더 리소스 바인딩 (필요 시)
		//ID3D12DescriptorHeap* heaps[] = { _cbvSrvUavHeap.Get() };
		//_commandList->SetDescriptorHeaps(_countof(heaps), heaps);
		//_commandList->SetGraphicsRootDescriptorTable(0, _gpuHandleStart);


		// 11) Draw call
		_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
		_commandList->IASetIndexBuffer(&_indexBufferView);
		_commandList->DrawIndexedInstanced(_indexCount, 1, 0, 0, 0);

		// 12) 렌더타깃 → present 상태 전환
		{
			const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				_renderTargets[_frameIndex].Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT);
			_commandList->ResourceBarrier(1, &barrier);
		}

		// 13) Command List Close
		THROW_IF_FAILED(_commandList->Close());
	}

	void RendererImpl::waitForPreviousFrame()
	{
		// 1) 현재 커맨드 큐에 시그널
		const UINT64 fenceToWait = _fenceValue;
		THROW_IF_FAILED(_commandQueue->Signal(_fence.Get(), fenceToWait));
		_fenceValue++;

		// 2) GPU가 fenceToWait 까지 실행 완료할 때까지 대기
		if (_fence->GetCompletedValue() < fenceToWait)
		{
			THROW_IF_FAILED(_fence->SetEventOnCompletion(fenceToWait, _fenceEvent));
			WaitForSingleObject(_fenceEvent, INFINITE);
		}

		// 3) 다음 프레임 인덱스 갱신
		_frameIndex = (_swapChain->GetCurrentBackBufferIndex());
	}
}