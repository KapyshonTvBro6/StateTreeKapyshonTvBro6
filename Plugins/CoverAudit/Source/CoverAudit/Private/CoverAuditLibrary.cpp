#include "CoverAuditLibrary.h"
#include "Modules/ModuleManager.h"
#include "StateTree.h"
#include "StateTreeSchema.h"
#include "StateTreeEditorData.h"
#include "StateTreeState.h"
#include "StateTreeCompiler.h"
#include "StateTreeCompilerLog.h"
#include "K2Node_VariableSet.h"
#include "K2Node_CallFunction.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "Components/SceneComponent.h"
#include "Components/StateTreeComponent.h"
IMPLEMENT_MODULE(FDefaultModuleImpl,CoverAudit)
bool UCoverAuditLibrary::RepairTree(UStateTree* Tree) {
 if(!Tree)return false; auto* Ed=Cast<UStateTreeEditorData>(Tree->EditorData); if(!Ed||!Ed->Schema)return false;
 Ed->Modify(); Tree->Modify();
 TFunction<void(UStateTreeState*)> Visit=[&](UStateTreeState* State){
  State->Modify();
  for(auto& Tr:State->Transitions)if(Tr.Trigger==EStateTreeTransitionTrigger::OnStateCompleted)Tr.Trigger=EStateTreeTransitionTrigger::OnStateSucceeded;
  for(auto& Task:State->Tasks)if(Task.InstanceObject){
   for(const auto& Desc:Ed->Schema->GetContextDataDescs()) {
    FName Prop=Desc.Name==TEXT("Actor")?FName(TEXT("Enemy")):Desc.Name==TEXT("AIController")?FName(TEXT("Controller")):NAME_None;
    if(Prop.IsNone()||!Task.InstanceObject->GetClass()->FindPropertyByName(Prop))continue;
    FPropertyBindingPath Src,Dst; Src.SetStructID(Desc.ID); Dst.SetStructID(Task.ID); Dst.FromString(Prop.ToString()); Ed->AddPropertyBinding(Src,Dst);
   }
  }
  for(auto Child:State->Children)Visit(Child);
 }; for(auto Root:Ed->SubTrees)Visit(Root);
 FStateTreeCompilerLog Log; FStateTreeCompiler Compiler(Log); bool Result=Compiler.Compile(*Tree); Log.DumpToLog(LogTemp); Tree->MarkPackageDirty();return Result;
}
UK2Node_VariableSet* UCoverAuditLibrary::AddVariableSet(UBlueprint* Blueprint, UEdGraph* Graph,FName VariableName,UClass* OwnerClass) {
 if(!Blueprint||!Graph)return nullptr; Blueprint->Modify(); Graph->Modify(); FGraphNodeCreator<UK2Node_VariableSet> Creator(*Graph); auto* Node=Creator.CreateNode();
 if(OwnerClass)Node->VariableReference.SetExternalMember(VariableName,OwnerClass);else Node->VariableReference.SetSelfMember(VariableName);
 Creator.Finalize(); FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);return Node;
}
UK2Node_CallFunction* UCoverAuditLibrary::AddCall(UBlueprint* Blueprint, UEdGraph* Graph,FName FunctionName,UClass* OwnerClass) {
 if(!Blueprint||!Graph||!OwnerClass)return nullptr; auto* Func=OwnerClass->FindFunctionByName(FunctionName);if(!Func)return nullptr;
 Blueprint->Modify();Graph->Modify();FGraphNodeCreator<UK2Node_CallFunction> Creator(*Graph); auto* Node=Creator.CreateNode();Node->SetFromFunction(Func);Creator.Finalize();FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);return Node;
}
bool UCoverAuditLibrary::MakeComponentRoot(UBlueprint* Blueprint,FName VariableName) {
 if(!Blueprint||!Blueprint->SimpleConstructionScript)return false;USimpleConstructionScript* SCS=Blueprint->SimpleConstructionScript;USCS_Node* Target=nullptr;
 for(auto Node:SCS->GetAllNodes())if(Node->GetVariableName()==VariableName)Target=Node;
 if(!Target)return false;USCS_Node* Old=nullptr;for(auto Root:SCS->GetRootNodes())if(Cast<USceneComponent>(Root->ComponentTemplate)){Old=Root;break;}if(!Old)return false;if(Old==Target)return true;
 Blueprint->Modify();SCS->Modify();Old->Modify();Target->Modify();Old->RemoveChildNode(Target,false);SCS->RemoveNode(Old,false);SCS->AddNode(Target);Target->AddChildNode(Old,false);FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);return true;
}
TArray<FName> UCoverAuditLibrary::GetActiveStates(UStateTreeComponent* Component) {return Component?Component->GetActiveStateNames():TArray<FName>();}

