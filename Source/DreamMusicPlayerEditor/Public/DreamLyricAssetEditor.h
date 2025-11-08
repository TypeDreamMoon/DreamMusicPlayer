#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "DreamLyricAsset.h"
#include "DreamLyricGroupWrapper.h"

class UDreamLyricAsset;
class IDetailsView;

// 前向声明
namespace FDreamLyricAssetEditor_Internal
{
	struct FLyricGroupDisplayData;
}

/**
 * @brief 歌词资产编辑器
 * 
 * 提供可视化的歌词编辑界面，支持编辑歌词组、行和单词
 */
class FDreamLyricAssetEditor : public FAssetEditorToolkit
{
public:
	/**
	 * @brief 初始化编辑器
	 * 
	 * @param Mode 编辑模式
	 * @param InitToolkitHost 工具包主机
	 * @param InLyricAsset 要编辑的歌词资产
	 */
	void InitLyricAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, UDreamLyricAsset* InLyricAsset);

	// FAssetEditorToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager) override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;

	// IToolkit interface
	virtual void SaveAsset_Execute() override;
	virtual bool OnRequestClose(EAssetEditorCloseReason InCloseReason) override;

private:
	/**
	 * @brief 创建属性详情视图
	 */
	TSharedRef<SDockTab> SpawnPropertiesTab(const FSpawnTabArgs& Args);

	/**
	 * @brief 创建歌词列表视图
	 */
	TSharedRef<SDockTab> SpawnLyricListTab(const FSpawnTabArgs& Args);

	/**
	 * @brief 创建统计信息视图
	 */
	TSharedRef<SDockTab> SpawnStatisticsTab(const FSpawnTabArgs& Args);

	/**
	 * @brief 创建编辑视图
	 */
	TSharedRef<SDockTab> SpawnEditTab(const FSpawnTabArgs& Args);

	/**
	 * @brief 刷新所有视图
	 */
	void RefreshAllViews();

	/**
	 * @brief 刷新歌词列表
	 */
	void RefreshLyricList();

	/**
	 * @brief 刷新统计信息
	 */
	void RefreshStatistics();

	/**
	 * @brief 生成歌词列表项
	 */
	TSharedRef<ITableRow> GenerateLyricListRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	// 前向声明结构体
	struct FLyricGroupDisplayData;

	/**
	 * @brief 生成歌词组列表项（新版本）
	 */
	TSharedRef<ITableRow> GenerateLyricGroupRow(TSharedPtr<FLyricGroupDisplayData> InGroupData, const TSharedRef<STableViewBase>& OwnerTable);

	/**
	 * @brief 创建歌词行内容
	 */
	TSharedRef<SWidget> CreateLyricLinesWidget(TSharedPtr<FLyricGroupDisplayData> InGroupData);

	/**
	 * @brief 获取角色显示颜色
	 */
	FLinearColor GetRoleColor(EDreamMusicLyricTextRole Role) const;

	/**
	 * @brief 获取角色显示名称
	 */
	FText GetRoleDisplayName(EDreamMusicLyricTextRole Role) const;

	/**
	 * @brief 处理搜索框文本变更
	 */
	void OnSearchTextChanged(const FText& InSearchText);

	/**
	 * @brief 添加新组
	 */
	FReply OnAddNewGroup();

	/**
	 * @brief 删除选中的组
	 */
	FReply OnDeleteSelectedGroup();

	/**
	 * @brief 删除选中的组（用于菜单，返回 void）
	 */
	void OnDeleteSelectedGroupMenu();

	/**
	 * @brief 排序组
	 */
	FReply OnSortGroups();

	/**
	 * @brief 编辑选中的组
	 */
	void OnEditSelectedGroup();

	/**
	 * @brief 构建列表项的右键菜单
	 */
	void BuildContextMenu(FMenuBuilder& MenuBuilder, TSharedPtr<FString> Item);

	/**
	 * @brief 刷新编辑视图
	 */
	void RefreshEditView();

	/**
	 * @brief 处理资产属性变更
	 */
	void OnPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);

	/**
	 * @brief 处理资产对象变更
	 */
	void OnObjectPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent);

	/**
	 * @brief 处理编辑组属性变更
	 */
	void OnEditGroupPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);

private:
	/** 要编辑的歌词资产 */
	TObjectPtr<UDreamLyricAsset> LyricAsset;

	/** 属性详情视图 */
	TSharedPtr<IDetailsView> DetailsView;

	/** 歌词列表视图 */
	TSharedPtr<SListView<TSharedPtr<FString>>> LyricListView;

	/** 搜索框 */
	TSharedPtr<class SSearchBox> SearchBox;

	/** 歌词列表数据 */
	TArray<TSharedPtr<FString>> LyricListItems;

	/** 歌词组数据结构 */
	struct FLyricGroupDisplayData
	{
		FDreamMusicLyricTimestamp Timestamp;
		TArray<TPair<EDreamMusicLyricTextRole, FString>> Lines; // 角色和文本的配对
		int32 GroupIndex;
	};

	/** 歌词组显示数据 */
	TArray<TSharedPtr<FLyricGroupDisplayData>> LyricGroupData;

	/** 选中的组索引 */
	int32 SelectedGroupIndex;

	/** 搜索框文本 */
	FText SearchText;

	/** 统计信息文本块 */
	TSharedPtr<class STextBlock> StatisticsTextBlock;

	/** 编辑视图的详情视图 */
	TSharedPtr<IDetailsView> EditDetailsView;

	/** 当前编辑的组对象（临时对象用于编辑） */
	UPROPERTY()
	TObjectPtr<UObject> EditingGroupObject;

	/** 属性变更委托句柄 */
	FDelegateHandle PropertyChangedHandle;
};

