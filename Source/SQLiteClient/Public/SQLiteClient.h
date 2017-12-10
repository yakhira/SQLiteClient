// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformProcess.h"
#include "sqlite/sqlite3.h"
#include "SQLiteClient.generated.h"

UENUM()
namespace SQLiteColumnTypes {
	enum Type {
         IO_INT                UMETA(DisplayName = "INT(11)"),
		 IO_INTEGER            UMETA(DisplayName = "INTEGER"),
         IO_VARCHAR            UMETA(DisplayName = "VARCHAR(255)"),
         IO_DATE               UMETA(DisplayName = "DATE"),
		 IO_DATETIME           UMETA(DisplayName = "DATETIME"),
		 IO_TEXT			   UMETA(DisplayName = "TEXT")
     };
}

USTRUCT(BlueprintType)
struct FValuesMap
{	
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SQLite Client")
    TMap<FString, FString> Map;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class USQLiteClient : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USQLiteClient();

	UFUNCTION(BlueprintCallable, Category = "SQLite Client")
    bool OpenSQLiteDB();

	UFUNCTION(BlueprintCallable, Category = "SQLite Client")
    TArray<FValuesMap> ExecSQLiteQuery(FString Query);	

	UFUNCTION(BlueprintCallable, Category = "SQLite Client")
	bool CreateSQLiteTable(FString TableName);

	UFUNCTION(BlueprintCallable, Category = "SQLite Client")
	bool AddSQLiteColumn(FString TableName, FString ColumnName, TEnumAsByte<SQLiteColumnTypes::Type> ColumnType);

	UFUNCTION(BlueprintCallable, Category = "SQLite Client")
	bool DeleteSQLiteTable(FString TableName);
	
	UFUNCTION(BlueprintCallable, Category = "SQLite Client")
	bool AddSQLiteData(FString TableName, UPARAM(ref) TArray<FString>& Fields, UPARAM(ref) TArray<FString>& Values);
	
	UFUNCTION(BlueprintCallable, Category = "SQLite Client")
	bool UpdateSQLiteData(FString TableName, FString ColumnName, FString NewValue, FString Condition);

	UFUNCTION(BlueprintCallable, Category = "SQLite Client")
	TArray<FValuesMap> SelectSQLiteData(FString TableName, FString Condition);

	UFUNCTION(BlueprintCallable, Category = "SQLite Client")
	bool DeleteSQLiteData(FString TableName, FString Condition);

	UPROPERTY(EditAnywhere, Category = "SQLite Client")
	FString DatabaseFile;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	FString GetSQLiteColumnType(int32 EnumValue);
	sqlite3 *Db = nullptr;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
	
};
