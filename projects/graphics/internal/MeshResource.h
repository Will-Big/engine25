#pragma once

namespace graphics
{
	namespace internal
	{
		class MeshResource
		{
		public:
			// 생성자: 정점·인덱스 데이터를 받고, 버퍼 생성과 업로드까지 처리
			MeshResource(ID3D12Device* device,
				ID3D12GraphicsCommandList* cmdList,
				const void* vertexData,
				UINT vertexCount,
				UINT vertexStride,
				const void* indexData, // 인덱스 데이터도 void*로 받을 수 있음
				UINT indexCount,
				DXGI_FORMAT indexFormat); // 인덱스 포맷 추가

			// 커맨드 리스트에 자신의 바인딩+Draw 호출
			void Bind(ID3D12GraphicsCommandList* cmdList) const;
			void Draw(ID3D12GraphicsCommandList* cmdList) const;

		private:
			// GPU 리소스
			Microsoft::WRL::ComPtr<ID3D12Resource>  _vb, _vbUpload;
			Microsoft::WRL::ComPtr<ID3D12Resource>  _ib, _ibUpload;

			// 뷰
			D3D12_VERTEX_BUFFER_VIEW				_vbView = {};
			D3D12_INDEX_BUFFER_VIEW					_ibView = {};

			UINT									_vertexCount = 0;
			UINT									_indexCount = 0;
		};
	}
}
