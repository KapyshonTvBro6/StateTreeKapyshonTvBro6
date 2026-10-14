#pragma once
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CoverAuditLibrary.generated.h"
class UStateTree; class UBlueprint; class UEdGraph; class UK2Node_VariableSet; class UK2Node_CallFunction;
class UStateTreeComponent;
UCLASS()
class COVERAUDIT_API UCoverAuditLibrary : public UBlueprintFunctionLibrary {
 GENERATED_BODY()
public:
 UFUNCTION(BlueprintCallable,Category="Editor Repair") static bool RepairTree(UStateTree* Tree);
 UFUNCTION(BlueprintCallable,Category="Editor Repair") static UK2Node_VariableSet* AddVariableSet(UBlueprint* Blueprint, UEdGraph* Graph,FName VariableName,UClass* OwnerClass);
 UFUNCTION(BlueprintCallable,Category="Editor Repair") static UK2Node_CallFunction* AddCall(UBlueprint* Blueprint, UEdGraph* Graph,FName FunctionName,UClass* OwnerClass);
 UFUNCTION(BlueprintCallable,Category="Editor Repair") static bool MakeComponentRoot(UBlueprint* Blueprint,FName VariableName);
 UFUNCTION(BlueprintCallable,Category="Editor Repair") static TArray<FName> GetActiveStates(UStateTreeComponent* Component);
};
