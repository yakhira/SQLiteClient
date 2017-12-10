// Fill out your copyright notice in the Description page of Project Settings.

#include "SQLiteClient.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSQLite, Log, All);
DEFINE_LOG_CATEGORY(LogSQLite);

// Sets default values for this component's properties
USQLiteClient::USQLiteClient()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
}

// Called when the game starts
void USQLiteClient::BeginPlay()
{
	DatabaseFile = DatabaseFile.Replace(TEXT("{UserDir}"), FGenericPlatformProcess::UserDir());
	DatabaseFile = DatabaseFile.Replace(TEXT("{UserTempDir}"), FGenericPlatformProcess::UserTempDir());
	DatabaseFile = DatabaseFile.Replace(TEXT("{UserName}"), FGenericPlatformProcess::UserName(FALSE));
	DatabaseFile = DatabaseFile.Replace(TEXT("{UserSettingsDir}"), FGenericPlatformProcess::UserSettingsDir());
	DatabaseFile = DatabaseFile.Replace(TEXT("{GameUserDir}"), *FPaths::GameUserDir());

	Super::BeginPlay();

	// ...
	
}
//FindObject<UEnum>(ANY_PACKAGE, TEXT("EKaoriGameUnit"), true)->GetDisplayNameText(State);
void USQLiteClient::EndPlay(const EEndPlayReason::Type EndPlayReason)
{

	sqlite3_close(Db);
	UE_LOG(LogSQLite, Log, TEXT("Database %s closed"), *DatabaseFile);
	Super::EndPlay(EndPlayReason);

	// ...
}

FString USQLiteClient::GetSQLiteColumnType(int32 EnumValue)
{
	switch(EnumValue)
	{
		case SQLiteColumnTypes::IO_INT: 
			return "INT(11)";
		break;
		case SQLiteColumnTypes::IO_INTEGER: 
			return "INTEGER";
		break;
		case SQLiteColumnTypes::IO_VARCHAR: 
			return "VARCHAR(255)";
		break;
		case SQLiteColumnTypes::IO_DATE: 
			return "DATE";
		break;
		case SQLiteColumnTypes::IO_DATETIME: 
			return "DATETIME";
		break;
		case SQLiteColumnTypes::IO_TEXT: 
			return "TEXT";
		break;
	}
	return "";
}

bool USQLiteClient::OpenSQLiteDB()
{
	FString DatabasePath = FPaths::GetPath(DatabaseFile);

	if (!DatabasePath.IsEmpty())
	{
		if (!FPaths::DirectoryExists(DatabasePath))
		{
			UE_LOG(LogSQLite, Error, TEXT("Path %s not found.\n"), *DatabasePath);
			return FALSE;
		}
	}

	const char *strDatabaseFile = TCHAR_TO_ANSI(*DatabaseFile);

	if (sqlite3_open(strDatabaseFile, &Db))
	{
		FString ErrorString = FString(UTF8_TO_TCHAR(sqlite3_errmsg(Db)));
		UE_LOG(LogSQLite, Error, TEXT("Error to open/create database %s: %s.\n"), *DatabaseFile, *ErrorString);
		return FALSE;
	}
	UE_LOG(LogSQLite, Log, TEXT("Database %s is ready.\n"), *DatabaseFile);
	return TRUE;
}

TArray<FValuesMap> USQLiteClient::ExecSQLiteQuery(FString Query)
{
	FValuesMap Values;
	TArray<FValuesMap> Result;

	char *Err = 0;
	sqlite3_stmt *Stmt = NULL;
	const char *strQuery = TCHAR_TO_ANSI(*Query);

    int rc = sqlite3_prepare_v2(Db, strQuery, -1, &Stmt, NULL);

    if (rc != SQLITE_OK)
	{
		UE_LOG(LogSQLite, Error, TEXT("Error execute query %s.\n"), *Query);
		return Result;
	}
	
  	rc = sqlite3_step(Stmt);
	
    while (rc != SQLITE_DONE && rc != SQLITE_OK)
    {
        for (int Index = 0; Index < sqlite3_column_count(Stmt); Index++)
        {
            int Type = sqlite3_column_type(Stmt, Index);
            FString ColumnName = FString(UTF8_TO_TCHAR(sqlite3_column_name(Stmt, Index)));

            if (Type == SQLITE_INTEGER)
            {
				Values.Map.Add(ColumnName, FString::FromInt(sqlite3_column_int(Stmt, Index)));
                
            }
            else if (Type == SQLITE_FLOAT)
            {
				Values.Map.Add(ColumnName, FString::SanitizeFloat(sqlite3_column_int(Stmt, Index)));
                
            }
            if (Type == SQLITE_TEXT)
            {
				Values.Map.Add(ColumnName, FString(UTF8_TO_TCHAR(sqlite3_column_text(Stmt, Index))));
            }
        }
        rc = sqlite3_step(Stmt);
		Result.Add(Values);
    }
    rc = sqlite3_finalize(Stmt);
	
	return Result;
}

bool USQLiteClient::CreateSQLiteTable(FString TableName)
{
	char *Err = 0;

	FString Query = "CREATE TABLE IF NOT EXISTS `" + TableName + "` (`id` INTEGER PRIMARY KEY AUTOINCREMENT)";

	const char *strQuery = TCHAR_TO_ANSI(*Query);

	if (sqlite3_exec(Db, strQuery, 0, 0, &Err))
	{
		FString ErrorString = FString(UTF8_TO_TCHAR(Err));
		UE_LOG(LogSQLite, Error, TEXT("Error create table %s: %s.\n"), *TableName, *ErrorString);
		sqlite3_free(Err);
		return FALSE;
	}
	UE_LOG(LogSQLite, Log, TEXT("Table %s successfully created.\n"), *TableName);
	return TRUE;
}

bool USQLiteClient::AddSQLiteColumn(FString TableName, FString ColumnName, TEnumAsByte<SQLiteColumnTypes::Type> ColumnType)
{
	char *Err = 0;

	FString Query = "ALTER TABLE `" + TableName + "` ADD COLUMN `" + ColumnName + "` " + GetSQLiteColumnType(ColumnType);

	const char *strQuery = TCHAR_TO_ANSI(*Query);

	if (sqlite3_exec(Db, strQuery, 0, 0, &Err))
	{
		FString ErrorString = FString(UTF8_TO_TCHAR(Err));
		UE_LOG(LogSQLite, Error, TEXT("Error to add column %s: %s.\n"), *ColumnName, *ErrorString);
		sqlite3_free(Err);
		return FALSE;
	}
	UE_LOG(LogSQLite, Log, TEXT("Column %s successfully added to table %s.\n"), *ColumnName, *TableName);
	
	return TRUE;
}

bool USQLiteClient::DeleteSQLiteTable(FString TableName)
{
	char *Err = 0;

	FString Query = "DROP TABLE " + TableName;
	
	const char *strQuery = TCHAR_TO_ANSI(*Query);

	if (sqlite3_exec(Db, strQuery, 0, 0, &Err))
	{
		FString ErrorString = FString(UTF8_TO_TCHAR(Err));
		UE_LOG(LogSQLite, Error, TEXT("Error to delete table %s: %s.\n"), *TableName, *ErrorString);
		sqlite3_free(Err);
		return FALSE;
	}
	UE_LOG(LogSQLite, Log, TEXT("Table %s successfully deleted.\n"), *TableName);
	return TRUE;
}
bool USQLiteClient::AddSQLiteData(FString TableName, UPARAM(ref) TArray<FString>& Fields, UPARAM(ref) TArray<FString>& Values)
{	
	FString FieldsString, ValuesString;
	FString Query = "INSERT INTO " + TableName + " (<FieldsString>) VALUES (<ValuesString>)";

	if (Fields.Num() != Values.Num())
	{	
		UE_LOG(LogSQLite, Error, TEXT("Error to add data to table %s, numbers of Fields not queal to numbers of Values.\n"), *TableName);
		return FALSE;
	}
	
	for (int Index = 0; Index < Fields.Num(); Index++)
	{
		FieldsString += "`" + Fields[Index] + "`,";
		ValuesString += "'" + Values[Index] + "',";
	}

	FieldsString.RemoveFromEnd(",");
	ValuesString.RemoveFromEnd(",");

	Query = Query.Replace(TEXT("<FieldsString>"), *FieldsString);
	Query = Query.Replace(TEXT("<ValuesString>"), *ValuesString);

	char *Err = 0;
	const char *strQuery = TCHAR_TO_ANSI(*Query);

	if (sqlite3_exec(Db, strQuery, 0, 0, &Err))
	{
		FString ErrorString = FString(UTF8_TO_TCHAR(Err));
		UE_LOG(LogSQLite, Error, TEXT("Error to add data to table %s: %s.\n"), *TableName, *ErrorString);
		sqlite3_free(Err);
		return FALSE;
	}

	UE_LOG(LogSQLite, Log, TEXT("Values successfully added to table %s.\n"), *TableName);
	return TRUE;
}

bool USQLiteClient::UpdateSQLiteData(FString TableName, FString ColumnName, FString NewValue, FString Condition)
{
	char *Err = 0;

	FString Query = "UPDATE `" + TableName + "` SET `" + ColumnName  + "`='" + NewValue + "'";

	if (!Condition.IsEmpty())
		Query += " WHERE " + Condition;

	const char *strQuery = TCHAR_TO_ANSI(*Query);

	if (sqlite3_exec(Db, strQuery, 0, 0, &Err))
	{
		FString ErrorString = FString(UTF8_TO_TCHAR(Err));
		UE_LOG(LogSQLite, Error, TEXT("Error to update data in table %s: %s.\n"), *TableName, *ErrorString);
		sqlite3_free(Err);
		return FALSE;
	}
	UE_LOG(LogSQLite, Log, TEXT("Data successfully updated in table %s.\n"), *TableName);

	return TRUE;
}

TArray<FValuesMap> USQLiteClient::SelectSQLiteData(FString TableName, FString Condition)
{
	FValuesMap Values;
	TArray<FValuesMap> Result;

	FString Query = "SELECT * FROM " + TableName;

	if (!Condition.IsEmpty())
		Query += " WHERE " + Condition;

	char *Err = 0;
	sqlite3_stmt *Stmt = NULL;
	const char *strQuery = TCHAR_TO_ANSI(*Query);

    int rc = sqlite3_prepare_v2(Db, strQuery, -1, &Stmt, NULL);

    if (rc != SQLITE_OK)
	{
		UE_LOG(LogSQLite, Error, TEXT("Error select data from table %s.\n"), *TableName);
		return Result;
	}
	
  	rc = sqlite3_step(Stmt);
	
    while (rc != SQLITE_DONE && rc != SQLITE_OK)
    {
        for (int Index = 0; Index < sqlite3_column_count(Stmt); Index++)
        {
            int Type = sqlite3_column_type(Stmt, Index);
            FString ColumnName = FString(UTF8_TO_TCHAR(sqlite3_column_name(Stmt, Index)));

            if (Type == SQLITE_INTEGER)
            {
				Values.Map.Add(ColumnName, FString::FromInt(sqlite3_column_int(Stmt, Index)));
                
            }
            else if (Type == SQLITE_FLOAT)
            {
				Values.Map.Add(ColumnName, FString::SanitizeFloat(sqlite3_column_int(Stmt, Index)));
                
            }
            if (Type == SQLITE_TEXT)
            {
				Values.Map.Add(ColumnName, FString(UTF8_TO_TCHAR(sqlite3_column_text(Stmt, Index))));
            }
        }
        rc = sqlite3_step(Stmt);
		Result.Add(Values);
    }
    rc = sqlite3_finalize(Stmt);
	
	return Result;
}

bool USQLiteClient::DeleteSQLiteData(FString TableName, FString Condition)
{
	char *Err = 0;

	FString Query = "DELETE FROM " + TableName;

	if (!Condition.IsEmpty())
		Query += " WHERE " + Condition;

	const char *strQuery = TCHAR_TO_ANSI(*Query);

	if (sqlite3_exec(Db, strQuery, 0, 0, &Err)){
		FString ErrorString = FString(UTF8_TO_TCHAR(Err));
		UE_LOG(LogSQLite, Error, TEXT("Error delete data from table %s: %s.\n"), *TableName, *ErrorString);
		sqlite3_free(Err);
		return FALSE;
	}
	UE_LOG(LogSQLite, Log, TEXT("Data successfully deleted from table %s.\n"), *TableName);
	return TRUE;
}

// Called every frame
void USQLiteClient::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

