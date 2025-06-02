#include "pch.h"
#include "MeshResource.h"

graphics::internal::MeshResource::MeshResource(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* vertexData,
                                               UINT vertexCount, UINT vertexStride, const void* indexData, UINT indexCount, DXGI_FORMAT indexFormat)
{
	_vertexCount = vertexCount;
	_indexCount = indexCount;

    // 1) 사이즈 계산
	const UINT vbSize = vertexCount * vertexStride;
    const UINT ibSize = indexCount * (indexFormat == DXGI_FORMAT_R16_UINT ? sizeof(UINT16) : sizeof(UINT32));

    // 2) GPU 기본 힙에 버퍼 생성 (Copy Dest)
    {
        CD3DX12_HEAP_PROPERTIES heapDefault(D3D12_HEAP_TYPE_DEFAULT);
        auto descVB = CD3DX12_RESOURCE_DESC::Buffer(vbSize);
        auto descIB = CD3DX12_RESOURCE_DESC::Buffer(ibSize);

        THROW_IF_FAILED(device->CreateCommittedResource(
            &heapDefault,
            D3D12_HEAP_FLAG_NONE,
            &descVB,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&_vb)));

        if (indexCount > 0) 
        {
            THROW_IF_FAILED(device->CreateCommittedResource(
                &heapDefault,
                D3D12_HEAP_FLAG_NONE,
                &descIB,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                IID_PPV_ARGS(&_ib)));
        }
    }

    // 3) 업로드 힙에 버퍼 생성 (Generic Read)
    {
        CD3DX12_HEAP_PROPERTIES heapUpload(D3D12_HEAP_TYPE_UPLOAD);
        auto descVB = CD3DX12_RESOURCE_DESC::Buffer(vbSize);

        THROW_IF_FAILED(device->CreateCommittedResource(
            &heapUpload,
            D3D12_HEAP_FLAG_NONE,
            &descVB,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&_vbUpload)));

        if (indexCount > 0) 
        {
            auto descIB = CD3DX12_RESOURCE_DESC::Buffer(ibSize);
            THROW_IF_FAILED(device->CreateCommittedResource(
                &heapUpload,
                D3D12_HEAP_FLAG_NONE,
                &descIB,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&_ibUpload)));
        }
    }

    // 4) 데이터 복사 (Upload → Default)
    {
        D3D12_SUBRESOURCE_DATA vbData{};
        vbData.pData = vertexData;
        vbData.RowPitch = vbSize; // 버퍼이므로 RowPitch와 SlicePitch는 전체 크기
        vbData.SlicePitch = vbSize;

        UpdateSubresources<1>(cmdList, _vb.Get(), _vbUpload.Get(), 0, 0, 1, &vbData);

        if (indexCount > 0) 
        {
            D3D12_SUBRESOURCE_DATA ibData{};
            ibData.pData = indexData;
            ibData.RowPitch = ibSize;
            ibData.SlicePitch = ibSize;
            UpdateSubresources<1>(cmdList, _ib.Get(), _ibUpload.Get(), 0, 0, 1, &ibData);
        }

        // 상태 전환
        std::vector<CD3DX12_RESOURCE_BARRIER> barriers;
        barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
            _vb.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
        if (indexCount > 0) 
        {
            barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
                _ib.Get(),
                D3D12_RESOURCE_STATE_COPY_DEST,
                D3D12_RESOURCE_STATE_INDEX_BUFFER));
        }
        cmdList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
    }

    // 5) 뷰 설정
    _vbView.BufferLocation = _vb->GetGPUVirtualAddress();
    _vbView.StrideInBytes = vertexStride; // 전달받은 stride 사용
    _vbView.SizeInBytes = vbSize;

    if (indexCount > 0) 
    {
        _ibView.BufferLocation = _ib->GetGPUVirtualAddress();
        _ibView.Format = indexFormat; // 전달받은 format 사용
        _ibView.SizeInBytes = ibSize;
    }
}

void graphics::internal::MeshResource::Bind(ID3D12GraphicsCommandList* cmdList) const
{
}

void graphics::internal::MeshResource::Draw(ID3D12GraphicsCommandList* cmdList) const
{
    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    cmdList->IASetVertexBuffers(0, 1, &_vbView);

    if (_indexCount > 0)
    {
        cmdList->IASetIndexBuffer(&_ibView);
        cmdList->DrawIndexedInstanced(_indexCount, 1, 0, 0, 0);
    }
    else if (_vertexCount > 0) // 인덱스 없이 정점 버퍼만으로 그리기
    {
        cmdList->DrawInstanced(_vertexCount, 1, 0, 0);
    }
}
