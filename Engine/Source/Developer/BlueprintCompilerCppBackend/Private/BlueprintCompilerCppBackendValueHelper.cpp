// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "BlueprintCompilerCppBackendModulePrivatePCH.h"
#include "BlueprintCompilerCppBackendUtils.h"
#include "Editor/UnrealEd/Public/Kismet2/StructureEditorUtils.h"
#include "Engine/InheritableComponentHandler.h"
#include "Engine/DynamicBlueprintBinding.h"

void FEmitDefaultValueHelper::OuterGenerate(FEmitterLocalContext& Context
	, const UProperty* Property
	, const FString& OuterPath
	, const uint8* DataContainer
	, const uint8* OptionalDefaultDataContainer
	, EPropertyAccessOperator AccessOperator
	, bool bAllowProtected)
{
	// Determine if the given property contains an instanced default subobject reference. We only get here if the values are not identical.
	auto IsInstancedSubobjectLambda = [&](int32 ArrayIndex) -> bool
	{
		if (auto ObjectProperty = Cast<UObjectProperty>(Property))
		{
			check(DataContainer);
			check(OptionalDefaultDataContainer);

			auto ObjectPropertyValue = ObjectProperty->GetObjectPropertyValue_InContainer(DataContainer, ArrayIndex);
			auto DefaultObjectPropertyValue = ObjectProperty->GetObjectPropertyValue_InContainer(OptionalDefaultDataContainer, ArrayIndex);
			if (ObjectPropertyValue && ObjectPropertyValue->IsDefaultSubobject() && DefaultObjectPropertyValue && DefaultObjectPropertyValue->IsDefaultSubobject() && ObjectPropertyValue->GetFName() == DefaultObjectPropertyValue->GetFName())
			{
				return true;
			}
		}

		return false;
	};

	if (Property->HasAnyPropertyFlags(CPF_EditorOnly | CPF_Transient))
	{
		UE_LOG(LogK2Compiler, Log, TEXT("FEmitDefaultValueHelper Skip EditorOnly or Transient property: %s"), *Property->GetPathName());
		return;
	}

	for (int32 ArrayIndex = 0; ArrayIndex < Property->ArrayDim; ++ArrayIndex)
	{
		if (!OptionalDefaultDataContainer
			|| (!Property->Identical_InContainer(DataContainer, OptionalDefaultDataContainer, ArrayIndex) && !IsInstancedSubobjectLambda(ArrayIndex)))
		{
			FString PathToMember;
			if (Property->HasAnyPropertyFlags(CPF_NativeAccessSpecifierPrivate) || (!bAllowProtected && Property->HasAnyPropertyFlags(CPF_NativeAccessSpecifierProtected)))
			{
				ensure(EPropertyAccessOperator::None != AccessOperator);
				
				const FString PropertyObject = Context.GenerateGetProperty(Property); //X::StaticClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(X, X))

				const uint32 CppTemplateTypeFlags = EPropertyExportCPPFlags::CPPF_CustomTypeName
					| EPropertyExportCPPFlags::CPPF_NoConst | EPropertyExportCPPFlags::CPPF_NoRef | EPropertyExportCPPFlags::CPPF_NoStaticArray
					| EPropertyExportCPPFlags::CPPF_BlueprintCppBackend;
				const FString TypeDeclaration = Context.ExportCppDeclaration(Property, EExportedDeclaration::Parameter, CppTemplateTypeFlags, true);

				const FString OperatorStr = (EPropertyAccessOperator::Dot == AccessOperator) ? TEXT("&") : TEXT("");
				const FString ContainerStr = (EPropertyAccessOperator::None == AccessOperator) ? TEXT("this") : FString::Printf(TEXT("%s(%s)"), *OperatorStr, *OuterPath);
				PathToMember = Context.GenerateUniqueLocalName();
				Context.AddLine(FString::Printf(TEXT("auto& %s = *(%s->ContainerPtrToValuePtr<%s>(%s, %d));")
					, *PathToMember, *PropertyObject, *TypeDeclaration, *ContainerStr, ArrayIndex));
			}
			else
			{
				const FString AccessOperatorStr = (EPropertyAccessOperator::None == AccessOperator) ? TEXT("")
					: ((EPropertyAccessOperator::Pointer == AccessOperator) ? TEXT("->") : TEXT("."));
				const bool bStaticArray = (Property->ArrayDim > 1);
				const FString ArrayPost = bStaticArray ? FString::Printf(TEXT("[%d]"), ArrayIndex) : TEXT("");
				PathToMember = FString::Printf(TEXT("%s%s%s%s"), *OuterPath, *AccessOperatorStr, *FEmitHelper::GetCppName(Property), *ArrayPost);
			}
			const uint8* ValuePtr = Property->ContainerPtrToValuePtr<uint8>(DataContainer, ArrayIndex);
			const uint8* DefaultValuePtr = OptionalDefaultDataContainer ? Property->ContainerPtrToValuePtr<uint8>(OptionalDefaultDataContainer, ArrayIndex) : nullptr;
			InnerGenerate(Context, Property, PathToMember, ValuePtr, DefaultValuePtr);
		}
	}
}

FString FEmitDefaultValueHelper::GenerateGetDefaultValue(const UUserDefinedStruct* Struct, const FGatherConvertedClassDependencies& Dependencies)
{
	check(Struct);
	const FString StructName = FEmitHelper::GetCppName(Struct);
	FString Result = FString::Printf(TEXT("\tstatic %s GetDefaultValue()\n\t{\n\t\t%s DefaultData__;\n"), *StructName, *StructName);

	FStructOnScope StructData(Struct);
	FStructureEditorUtils::Fill_MakeStructureDefaultValue(Struct, StructData.GetStructMemory());

	FEmitterLocalContext Context(Dependencies);
	Context.IncreaseIndent();
	Context.IncreaseIndent();
	for (auto Property : TFieldRange<const UProperty>(Struct))
	{
		OuterGenerate(Context, Property, TEXT("DefaultData__"), StructData.GetStructMemory(), nullptr, EPropertyAccessOperator::Dot);
	}
	Result += Context.GetResult();
	Result += TEXT("\n\t\treturn DefaultData__;\n\t}\n");
	return Result;
}

void FEmitDefaultValueHelper::InnerGenerate(FEmitterLocalContext& Context, const UProperty* Property, const FString& PathToMember, const uint8* ValuePtr, const uint8* DefaultValuePtr, bool bWithoutFirstConstructionLine)
{
	auto OneLineConstruction = [](FEmitterLocalContext& LocalContext, const UProperty* LocalProperty, const uint8* LocalValuePtr, FString& OutSingleLine, bool bGenerateEmptyStructConstructor) -> bool
	{
		bool bComplete = true;
		FString ValueStr = HandleSpecialTypes(LocalContext, LocalProperty, LocalValuePtr);
		if (ValueStr.IsEmpty())
		{
			ValueStr = LocalContext.ExportTextItem(LocalProperty, LocalValuePtr);
			auto StructProperty = Cast<const UStructProperty>(LocalProperty);
			if (ValueStr.IsEmpty() && StructProperty)
			{
				check(StructProperty->Struct);
				if (bGenerateEmptyStructConstructor)
				{
					ValueStr = FString::Printf(TEXT("%s{}"), *FEmitHelper::GetCppName(StructProperty->Struct)); //don;t override existing values
				}
				bComplete = false;
			}
			else if (ValueStr.IsEmpty())
			{
				UE_LOG(LogK2Compiler, Error, TEXT("FEmitDefaultValueHelper Cannot generate initilization: %s"), *LocalProperty->GetPathName());
			}
		}
		OutSingleLine += ValueStr;
		return bComplete;
	};

	auto StructProperty = Cast<const UStructProperty>(Property);
	check(!StructProperty || StructProperty->Struct);
	auto ArrayProperty = Cast<const UArrayProperty>(Property);
	check(!ArrayProperty || ArrayProperty->Inner);

	if (!bWithoutFirstConstructionLine)
	{
		FString ValueStr;
		const bool bComplete = OneLineConstruction(Context, Property, ValuePtr, ValueStr, false);
		if (!ValueStr.IsEmpty())
		{
			Context.AddLine(FString::Printf(TEXT("%s = %s;"), *PathToMember, *ValueStr));
		}
		// array initialization "array_var = TArray<..>()" is complete, but it still needs items.
		if (bComplete && !ArrayProperty)
		{
			return;
		}
	}

	if (StructProperty)
	{
		for (auto LocalProperty : TFieldRange<const UProperty>(StructProperty->Struct))
		{
			OuterGenerate(Context, LocalProperty, PathToMember, ValuePtr, DefaultValuePtr, EPropertyAccessOperator::Dot);
		}
	}
	
	if (ArrayProperty)
	{
		FScriptArrayHelper ScriptArrayHelper(ArrayProperty, ValuePtr);
		for (int32 Index = 0; Index < ScriptArrayHelper.Num(); ++Index)
		{
			const uint8* LocalValuePtr = ScriptArrayHelper.GetRawPtr(Index);

			FString ValueStr;
			const bool bComplete = OneLineConstruction(Context, ArrayProperty->Inner, LocalValuePtr, ValueStr, true);
			Context.AddLine(FString::Printf(TEXT("%s.Add(%s);"), *PathToMember, *ValueStr));
			if (!bComplete)
			{
				const FString LocalPathToMember = FString::Printf(TEXT("%s[%d]"), *PathToMember, Index);
				InnerGenerate(Context, ArrayProperty->Inner, LocalPathToMember, LocalValuePtr, nullptr, true);
			}
		}
	}
}

FString FEmitDefaultValueHelper::HandleSpecialTypes(FEmitterLocalContext& Context, const UProperty* Property, const uint8* ValuePtr)
{
	//TODO: Use Path maps for Objects
	if (auto ObjectProperty = Cast<UObjectProperty>(Property))
	{
		UObject* Object = ObjectProperty->GetPropertyValue(ValuePtr);
		if (Object)
		{
			{
				UClass* ObjectClassToUse = Context.GetNativeOrConvertedClass(ObjectProperty->PropertyClass);
				const FString MappedObject = Context.FindGloballyMappedObject(Object, ObjectClassToUse);
				if (!MappedObject.IsEmpty())
				{
					return MappedObject;
				}
			}

			const bool bCreatingSubObjectsOfClass = (Context.CurrentCodeType == FEmitterLocalContext::EGeneratedCodeType::SubobjectsOfClass);
			{
				auto BPGC = Context.GetCurrentlyGeneratedClass();
				auto CDO = BPGC ? BPGC->GetDefaultObject(false) : nullptr;
				if (BPGC && Object && CDO && Object->IsIn(BPGC) && !Object->IsIn(CDO) && bCreatingSubObjectsOfClass)
				{
					return HandleClassSubobject(Context, Object, FEmitterLocalContext::EClassSubobjectList::MiscConvertedSubobjects);
				}
			}

			if (!bCreatingSubObjectsOfClass && Property->HasAnyPropertyFlags(CPF_InstancedReference))
			{
				const FString CreateAsInstancedSubobject = HandleInstancedSubobject(Context, Object, Object->HasAnyFlags(RF_ArchetypeObject));
				if (!CreateAsInstancedSubobject.IsEmpty())
				{
					return CreateAsInstancedSubobject;
				}
			}
		}
		else if (ObjectProperty->HasMetaData(FBlueprintMetadata::MD_LatentCallbackTarget))
		{
			return TEXT("this");
		}
	}

	if (auto StructProperty = Cast<UStructProperty>(Property))
	{
		if (TBaseStructure<FTransform>::Get() == StructProperty->Struct)
		{
			check(ValuePtr);
			const FTransform* Transform = reinterpret_cast<const FTransform*>(ValuePtr);
			const auto Rotation = Transform->GetRotation();
			const auto Translation = Transform->GetTranslation();
			const auto Scale = Transform->GetScale3D();
			return FString::Printf(TEXT("FTransform(FQuat(%f, %f, %f, %f), FVector(%f, %f, %f), FVector(%f, %f, %f))")
				, Rotation.X, Rotation.Y, Rotation.Z, Rotation.W
				, Translation.X, Translation.Y, Translation.Z
				, Scale.X, Scale.Y, Scale.Z);
		}

		if (TBaseStructure<FVector>::Get() == StructProperty->Struct)
		{
			const FVector* Vector = reinterpret_cast<const FVector*>(ValuePtr);
			return FString::Printf(TEXT("FVector(%f, %f, %f)"), Vector->X, Vector->Y, Vector->Z);
		}

		if (TBaseStructure<FGuid>::Get() == StructProperty->Struct)
		{
			const FGuid* Guid = reinterpret_cast<const FGuid*>(ValuePtr);
			return FString::Printf(TEXT("FGuid(0x%08X, 0x%08X, 0x%08X, 0x%08X)"), Guid->A, Guid->B, Guid->C, Guid->D);
		}
	}
	return FString();
}

FString FEmitDefaultValueHelper::HandleNonNativeComponent(FEmitterLocalContext& Context, const USCS_Node* Node, TSet<const UProperty*>& OutHandledProperties, TArray<FString>& NativeCreatedComponentProperties, const USCS_Node* ParentNode)
{
	check(Node);
	check(Context.CurrentCodeType == FEmitterLocalContext::EGeneratedCodeType::CommonConstructor);

	FString NativeVariablePropertyName;
	UBlueprintGeneratedClass* BPGC = CastChecked<UBlueprintGeneratedClass>(Context.GetCurrentlyGeneratedClass());
	if (UActorComponent* ComponentTemplate = Node->GetActualComponentTemplate(BPGC))
	{
		const FString VariableCleanName = Node->VariableName.ToString();

		const UObjectProperty* VariableProperty = FindField<UObjectProperty>(BPGC, *VariableCleanName);
		if (VariableProperty)
		{
			NativeVariablePropertyName = FEmitHelper::GetCppName(VariableProperty);
			OutHandledProperties.Add(VariableProperty);
		}
		else
		{
			NativeVariablePropertyName = VariableCleanName;
		}

		Context.AddCommonSubObject_InConstructor(ComponentTemplate, NativeVariablePropertyName);

		if (ComponentTemplate->GetOuter() == BPGC)
		{
			UClass* ComponentClass = ComponentTemplate->GetClass();
			check(ComponentClass != nullptr);

			UObject* ObjectToCompare = ComponentClass->GetDefaultObject(false);

			if (ComponentTemplate->HasAnyFlags(RF_InheritableComponentTemplate))
			{
				ObjectToCompare = Node->GetActualComponentTemplate(Cast<UBlueprintGeneratedClass>(BPGC->GetSuperClass()));
			}
			else
			{
				Context.AddHighPriorityLine(FString::Printf(TEXT("%s%s = CreateDefaultSubobject<%s>(TEXT(\"%s\"));")
					, (VariableProperty == nullptr) ? TEXT("auto ") : TEXT("")
					, *NativeVariablePropertyName
					, *FEmitHelper::GetCppName(ComponentClass)
					, *VariableCleanName));

				Context.AddLine(FString::Printf(TEXT("%s->CreationMethod = EComponentCreationMethod::Native;"), *NativeVariablePropertyName));
				NativeCreatedComponentProperties.Add(NativeVariablePropertyName);

				FString ParentVariableName;
				if (ParentNode)
				{
					const FString CleanParentVariableName = ParentNode->VariableName.ToString();
					const UObjectProperty* ParentVariableProperty = FindField<UObjectProperty>(BPGC, *CleanParentVariableName);
					ParentVariableName = ParentVariableProperty ? FEmitHelper::GetCppName(ParentVariableProperty) : CleanParentVariableName;
				}
				else if (USceneComponent* ParentComponentTemplate = Node->GetParentComponentTemplate(CastChecked<UBlueprint>(BPGC->ClassGeneratedBy)))
				{
					ParentVariableName = Context.FindGloballyMappedObject(ParentComponentTemplate, USceneComponent::StaticClass());
				}

				if (!ParentVariableName.IsEmpty())
				{
					Context.AddLine(FString::Printf(TEXT("%s->AttachParent = %s;"), *NativeVariablePropertyName, *ParentVariableName));
				}
			}

			for (auto Property : TFieldRange<const UProperty>(ComponentClass))
			{
				OuterGenerate(Context, Property, NativeVariablePropertyName
					, reinterpret_cast<const uint8*>(ComponentTemplate)
					, reinterpret_cast<const uint8*>(ObjectToCompare)
					, EPropertyAccessOperator::Pointer);
			}
		}
	}

	// Recursively handle child nodes.
	for (auto ChildNode : Node->ChildNodes)
	{
		HandleNonNativeComponent(Context, ChildNode, OutHandledProperties, NativeCreatedComponentProperties, Node);
	}

	return NativeVariablePropertyName;
}

struct FDependenciesHelper
{
	static void AddDependenciesInConstructor(FEmitterLocalContext& Context)
	{
		if (Context.Dependencies.ConvertedClasses.Num())
		{
			Context.AddLine(TEXT("// List of all referenced converted classes"));
		}
		for (auto LocStruct : Context.Dependencies.ConvertedClasses)
		{
			Context.AddLine(FString::Printf(TEXT("CastChecked<UDynamicClass>(GetClass())->ReferencedConvertedFields.Add(%s::StaticClass());")
				, *FEmitHelper::GetCppName(LocStruct, true)));
		}

		if (Context.Dependencies.ConvertedStructs.Num())
		{
			Context.AddLine(TEXT("// List of all referenced converted structures"));
		}
		for (auto LocStruct : Context.Dependencies.ConvertedStructs)
		{
			Context.AddLine(FString::Printf(TEXT("CastChecked<UDynamicClass>(GetClass())->ReferencedConvertedFields.Add(%s::StaticStruct());")
				, *FEmitHelper::GetCppName(LocStruct)));
		}

		if (Context.Dependencies.Assets.Num())
		{
			Context.AddLine(TEXT("// List of all referenced assets"));
		}
		for (auto LocAsset : Context.Dependencies.Assets)
		{
			const FString AssetStr = Context.FindGloballyMappedObject(LocAsset, nullptr, true, false);
			Context.AddLine(FString::Printf(TEXT("CastChecked<UDynamicClass>(GetClass())->UsedAssets.Add(%s);"), *AssetStr));
		}
	}

	static void AddStaticFunctionsForDependencies(FEmitterLocalContext& Context)
	{
		auto OriginalClass = Context.GetCurrentlyGeneratedClass();
		const FString CppClassName = FEmitHelper::GetCppName(OriginalClass);

		// __StaticDependenciesAssets
		Context.AddLine(FString::Printf(TEXT("void %s::__StaticDependenciesAssets(TArray<FName>& OutPackagePaths)"), *CppClassName));
		Context.AddLine(TEXT("{"));
		Context.IncreaseIndent();

		for (UObject* LocAsset : Context.Dependencies.Assets)
		{
			const FString PakagePath = LocAsset->GetOutermost()->GetPathName();
			Context.AddLine(FString::Printf(TEXT("OutPackagePaths.Add(TEXT(\"%s\"));"), *PakagePath));
		}

		Context.DecreaseIndent();
		Context.AddLine(TEXT("}"));

		// Register Helper Struct
		const FString RegisterHelperName = FString::Printf(TEXT("FRegisterHelper__%s"), *CppClassName);
		Context.AddLine(FString::Printf(TEXT("struct %s"), *RegisterHelperName));
		Context.AddLine(TEXT("{"));
		Context.IncreaseIndent();

		{
			// __StaticDependenciesAssets
			Context.DecreaseIndent();
			Context.AddLine(TEXT("#if WITH_EDITOR"));
			Context.IncreaseIndent();
			Context.AddLine(TEXT("static UField* __StaticCreateClass()"));
			Context.AddLine(TEXT("{"));
			Context.IncreaseIndent();
			Context.AddLine(FString::Printf(TEXT("return CastChecked<UDynamicClass>(%s::StaticClass());"), *FEmitHelper::GetCppName(OriginalClass, true)));
			Context.DecreaseIndent();
			Context.AddLine(TEXT("}"));
			Context.DecreaseIndent();
			Context.AddLine(TEXT("#endif //WITH_EDITOR"));
			Context.IncreaseIndent();
		}

		Context.AddLine(FString::Printf(TEXT("%s()"), *RegisterHelperName));
		Context.AddLine(TEXT("{"));
		Context.IncreaseIndent();

		Context.AddLine(FString::Printf(
			TEXT("FConvertedBlueprintsDependencies::Get().RegisterClass(TEXT(\"%s\"), &%s::__StaticDependenciesAssets);")
			, *OriginalClass->GetName()
			, *CppClassName
			, *CppClassName));

		{
			Context.DecreaseIndent();
			Context.AddLine(TEXT("#if WITH_EDITOR"));
			Context.IncreaseIndent();

			Context.AddLine(TEXT("FConvertedBlueprintsDependencies::Get().RegisterClass(&__StaticCreateClass);"));

			Context.DecreaseIndent();
			Context.AddLine(TEXT("#endif //WITH_EDITOR"));
			Context.IncreaseIndent();
		}

		Context.DecreaseIndent();
		Context.AddLine(TEXT("}"));

		Context.AddLine(FString::Printf(TEXT("static %s Instance;"), *RegisterHelperName));

		Context.DecreaseIndent();
		Context.AddLine(TEXT("};"));

		Context.AddLine(FString::Printf(TEXT("%s %s::Instance;"), *RegisterHelperName, *RegisterHelperName));
	}
};

FString FEmitDefaultValueHelper::GenerateConstructor(FEmitterLocalContext& Context)
{
	auto BPGC = CastChecked<UBlueprintGeneratedClass>(Context.GetCurrentlyGeneratedClass());
	const FString CppClassName = FEmitHelper::GetCppName(BPGC);

	Context.AddLine(FString::Printf(TEXT("%s::%s(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)"), *CppClassName, *CppClassName));
	Context.AddLine(TEXT("{"));
	Context.IncreaseIndent();

	// When CDO is created create all subobjects owned by the class
	{
		TArray<UActorComponent*> ActorComponentTempatesOwnedByClass = BPGC->ComponentTemplates;
		// Gather all CT from SCS and IH, the remaining ones are generated for class..
		if (auto SCS = BPGC->SimpleConstructionScript)
		{
			for (auto Node : SCS->GetAllNodes())
			{
				ActorComponentTempatesOwnedByClass.RemoveSwap(Node->ComponentTemplate);
			}
		}
		if (auto IH = BPGC->GetInheritableComponentHandler())
		{
			TArray<UActorComponent*> AllTemplates;
			IH->GetAllTemplates(AllTemplates);
			ActorComponentTempatesOwnedByClass.RemoveAllSwap([&](UActorComponent* Component) -> bool
			{
				return AllTemplates.Contains(Component);
			});
		}

		Context.AddLine(FString::Printf(TEXT("if(HasAnyFlags(RF_ClassDefaultObject) && (%s::StaticClass() == GetClass()))"), *CppClassName));
		Context.AddLine(TEXT("{"));
		Context.IncreaseIndent();
		Context.AddLine(TEXT("ensure(0 == CastChecked<UDynamicClass>(GetClass())->MiscConvertedSubobjects.Num());"));
		Context.AddLine(TEXT("ensure(0 == CastChecked<UDynamicClass>(GetClass())->ReferencedConvertedFields.Num());"));
		Context.AddLine(TEXT("ensure(0 == CastChecked<UDynamicClass>(GetClass())->UsedAssets.Num());"));
		Context.AddLine(TEXT("ensure(0 == CastChecked<UDynamicClass>(GetClass())->DynamicBindingObjects.Num());"));
		Context.AddLine(TEXT("ensure(0 == CastChecked<UDynamicClass>(GetClass())->ComponentTemplates.Num());"));
		Context.AddLine(TEXT("ensure(0 == CastChecked<UDynamicClass>(GetClass())->Timelines.Num());"));
		Context.CurrentCodeType = FEmitterLocalContext::EGeneratedCodeType::SubobjectsOfClass;
		Context.FlushLines();
		for (auto ComponentTemplate : ActorComponentTempatesOwnedByClass)
		{
			if (ComponentTemplate)
			{
				HandleClassSubobject(Context, ComponentTemplate, FEmitterLocalContext::EClassSubobjectList::ComponentTemplates);
			}
		}

		for (auto TimelineTemplate : BPGC->Timelines)
		{
			if (TimelineTemplate)
			{
				HandleClassSubobject(Context, TimelineTemplate, FEmitterLocalContext::EClassSubobjectList::Timelines);
			}
		}

		for (auto DynamicBindingObject : BPGC->DynamicBindingObjects)
		{
			if (DynamicBindingObject)
			{
				HandleClassSubobject(Context, DynamicBindingObject, FEmitterLocalContext::EClassSubobjectList::DynamicBindingObjects);
			}
		}

		FBackendHelperUMG::CreateClassSubobjects(Context);
		FDependenciesHelper::AddDependenciesInConstructor(Context);

		Context.DecreaseIndent();
		Context.AddLine(TEXT("}"));
	}

	// Components that must be fixed after serialization
	TArray<FString> NativeCreatedComponentProperties;

	{
		Context.CurrentCodeType = FEmitterLocalContext::EGeneratedCodeType::CommonConstructor;
		// Let's have an easy access to generated class subobjects
		Context.AddLine(TEXT("{")); // no shadow variables
		Context.IncreaseIndent();
		Context.FlushLines();

		UObject* CDO = BPGC->GetDefaultObject(false);

		UObject* ParentCDO = BPGC->GetSuperClass()->GetDefaultObject(false);
		check(CDO && ParentCDO);
		Context.AddLine(TEXT(""));

		FString NativeRootComponentFallback;
		TSet<const UProperty*> HandledProperties;

		// Generate ctor init code for native class default subobjects that are always instanced (e.g. components).
		// @TODO (pkavan) - We can probably make this faster by generating code to index through the DSO array instead (i.e. in place of HandleInstancedSubobject which will generate a lookup call per DSO).
		TArray<UObject*> NativeDefaultObjectSubobjects;
		BPGC->GetDefaultObjectSubobjects(NativeDefaultObjectSubobjects);
		for (auto DSO : NativeDefaultObjectSubobjects)
		{
			if (DSO && DSO->GetClass()->HasAnyClassFlags(CLASS_DefaultToInstanced))
			{
				// Determine if this is an editor-only subobject.
				bool bIsEditorOnlySubobject = false;
				if (const UActorComponent* ActorComponent = Cast<UActorComponent>(DSO))
				{
					bIsEditorOnlySubobject = ActorComponent->IsEditorOnly();
				}

				// Skip ctor code gen for editor-only subobjects, since they won't be used by the runtime. Any dependencies on editor-only subobjects will be handled later (see HandleInstancedSubobject).
				if (!bIsEditorOnlySubobject)
				{
					const FString VariableName = HandleInstancedSubobject(Context, DSO, false, true);

					// Keep track of which component can be used as a root, in case it's not explicitly set.
					if (NativeRootComponentFallback.IsEmpty())
					{
						USceneComponent* SceneComponent = Cast<USceneComponent>(DSO);
						if (SceneComponent && !SceneComponent->AttachParent && SceneComponent->CreationMethod == EComponentCreationMethod::Native)
						{
							NativeRootComponentFallback = VariableName;
						}
					}
				}
			}
		}

		// Check for a valid RootComponent property value; mark it as handled if already set in the defaults.
		bool bNeedsRootComponentAssignment = false;
		static const FName RootComponentPropertyName(TEXT("RootComponent"));
		const UObjectProperty* RootComponentProperty = FindField<UObjectProperty>(BPGC, RootComponentPropertyName);
		if (RootComponentProperty)
		{
			if (RootComponentProperty->GetObjectPropertyValue_InContainer(CDO))
			{
				HandledProperties.Add(RootComponentProperty);
			}
			else if (!NativeRootComponentFallback.IsEmpty())
			{
				Context.AddLine(FString::Printf(TEXT("RootComponent = %s;"), *NativeRootComponentFallback));
				HandledProperties.Add(RootComponentProperty);
			}
			else
			{
				bNeedsRootComponentAssignment = true;
			}
		}

		// Generate ctor init code for the SCS node hierarchy (i.e. non-native components). SCS nodes may have dependencies on native DSOs, but not vice-versa.
		TArray<const UBlueprintGeneratedClass*> BPGCStack;
		const bool bErrorFree = UBlueprintGeneratedClass::GetGeneratedClassesHierarchy(BPGC, BPGCStack);
		if (bErrorFree)
		{
			// Start at the base of the hierarchy so that dependencies are handled first.
			for (int32 i = BPGCStack.Num() - 1; i >= 0; --i)
			{
				if (BPGCStack[i]->SimpleConstructionScript)
				{
					for (auto Node : BPGCStack[i]->SimpleConstructionScript->GetRootNodes())
					{
						if (Node)
						{
							const FString NativeVariablePropertyName = HandleNonNativeComponent(Context, Node, HandledProperties, NativeCreatedComponentProperties);

							if (i == 0 && bNeedsRootComponentAssignment && Node->ComponentTemplate && Node->ComponentTemplate->IsA<USceneComponent>() && !NativeVariablePropertyName.IsEmpty())
							{
								Context.AddLine(FString::Printf(TEXT("RootComponent = %s;"), *NativeVariablePropertyName));

								bNeedsRootComponentAssignment = false;
								HandledProperties.Add(RootComponentProperty);
							}
						}
					}
				}
			}
		}

		// Generate ctor init code for generated Blueprint class property values that may differ from parent class defaults (or that otherwise belong to the generated Blueprint class).
		for (auto Property : TFieldRange<const UProperty>(BPGC))
		{
			const bool bNewProperty = Property->GetOwnerStruct() == BPGC;
			const bool bIsAccessible = bNewProperty || !Property->HasAnyPropertyFlags(CPF_NativeAccessSpecifierPrivate);
			if (bIsAccessible && !HandledProperties.Contains(Property))
			{
				OuterGenerate(Context, Property, TEXT(""), reinterpret_cast<const uint8*>(CDO), bNewProperty ? nullptr : reinterpret_cast<const uint8*>(ParentCDO), EPropertyAccessOperator::None, true);
			}
		}

		Context.DecreaseIndent();
		Context.AddLine(TEXT("}"));
	}
	Context.DecreaseIndent();
	Context.AddLine(TEXT("}"));
	Context.CurrentCodeType = FEmitterLocalContext::EGeneratedCodeType::Regular;

	{
		Context.AddLine(FString::Printf(TEXT("void %s::PostLoadSubobjects(FObjectInstancingGraph* OuterInstanceGraph)"), *CppClassName));
		Context.AddLine(TEXT("{"));
		Context.IncreaseIndent();
		Context.AddLine(TEXT("Super::PostLoadSubobjects(OuterInstanceGraph);"));
		
		for (auto& ComponentToFix : NativeCreatedComponentProperties)
		{
			Context.AddLine(FString::Printf(TEXT("if(ensure(%s))"), *ComponentToFix));
			Context.AddLine(TEXT("{"));
			Context.IncreaseIndent();
			Context.AddLine(FString::Printf(TEXT("%s->CreationMethod = EComponentCreationMethod::Native;"), *ComponentToFix));
			Context.DecreaseIndent();
			Context.AddLine(TEXT("}"));
		}
		Context.DecreaseIndent();
		Context.AddLine(TEXT("}"));
	}

	FDependenciesHelper::AddStaticFunctionsForDependencies(Context);

	FBackendHelperUMG::EmitWidgetInitializationFunctions(Context);

	const FString Result = Context.GetResult();
	Context.ClearResult();
	return Result;
}

FString FEmitDefaultValueHelper::HandleClassSubobject(FEmitterLocalContext& Context, UObject* Object, FEmitterLocalContext::EClassSubobjectList ListOfSubobjectsType)
{
	ensure(Context.CurrentCodeType == FEmitterLocalContext::EGeneratedCodeType::SubobjectsOfClass);
	const FString OuterStr = Context.FindGloballyMappedObject(Object->GetOuter());
	if (OuterStr.IsEmpty())
	{
		ensure(false);
		return FString();
	}

	const bool AddAsSubobjectOfClass = Object->GetOuter() == Context.GetCurrentlyGeneratedClass();
	const FString LocalNativeName = Context.GenerateUniqueLocalName();
	Context.AddClassSubObject_InConstructor(Object, LocalNativeName);
	UClass* ObjectClass = Object->GetClass();
	Context.AddHighPriorityLine(FString::Printf(
		TEXT("auto %s = NewObject<%s>(%s, TEXT(\"%s\"));")
		, *LocalNativeName
		, *FEmitHelper::GetCppName(ObjectClass)
		, *OuterStr
		, *Object->GetName()));
	if (AddAsSubobjectOfClass)
	{
		Context.RegisterClassSubobject(Object, ListOfSubobjectsType);
		Context.AddHighPriorityLine(FString::Printf(TEXT("CastChecked<UDynamicClass>(GetClass())->%s.Add(%s);")
			, Context.ClassSubobjectListName(ListOfSubobjectsType)
			, *LocalNativeName));
	}

	for (auto Property : TFieldRange<const UProperty>(ObjectClass))
	{
		OuterGenerate(Context, Property, LocalNativeName
			, reinterpret_cast<const uint8*>(Object)
			, reinterpret_cast<const uint8*>(ObjectClass->GetDefaultObject(false))
			, EPropertyAccessOperator::Pointer);
	}

	return LocalNativeName;
}

FString FEmitDefaultValueHelper::HandleInstancedSubobject(FEmitterLocalContext& Context, UObject* Object, bool bCreateInstance, bool bSkipEditorOnlyCheck)
{
	check(Object);

	// Make sure we don't emit initialization code for the same object more than once.
	FString LocalNativeName = Context.FindGloballyMappedObject(Object);
	if (!LocalNativeName.IsEmpty())
	{
		return LocalNativeName;
	}
	else
	{
		LocalNativeName = Context.GenerateUniqueLocalName();
	}

	if (Context.CurrentCodeType == FEmitterLocalContext::EGeneratedCodeType::SubobjectsOfClass)
	{
		Context.AddClassSubObject_InConstructor(Object, LocalNativeName);
	}
	else if (Context.CurrentCodeType == FEmitterLocalContext::EGeneratedCodeType::CommonConstructor)
	{
		Context.AddCommonSubObject_InConstructor(Object, LocalNativeName);
	}

	UClass* ObjectClass = Object->GetClass();

	// Determine if this is an editor-only subobject. When handling as a dependency, we'll create a "dummy" object in its place (below).
	bool bIsEditorOnlySubobject = false;
	if (!bSkipEditorOnlyCheck)
	{
		if (UActorComponent* ActorComponent = Cast<UActorComponent>(Object))
		{
			bIsEditorOnlySubobject = ActorComponent->IsEditorOnly();
			if (bIsEditorOnlySubobject)
			{
				// Replace the potentially editor-only class with a base actor/scene component class that's available to the runtime. We'll create a "dummy" object of this type to stand in for the editor-only subobject below.
				ObjectClass = ObjectClass->IsChildOf<USceneComponent>() ? USceneComponent::StaticClass() : UActorComponent::StaticClass();
			}
		}
	}

	auto BPGC = Context.GetCurrentlyGeneratedClass();
	auto CDO = BPGC ? BPGC->GetDefaultObject(false) : nullptr;
	if (!bIsEditorOnlySubobject && ensure(CDO) && (CDO == Object->GetOuter()))
	{
		if (bCreateInstance)
		{
			Context.AddHighPriorityLine(FString::Printf(TEXT("auto %s = CreateDefaultSubobject<%s>(TEXT(\"%s\"));")
				, *LocalNativeName, *FEmitHelper::GetCppName(ObjectClass), *Object->GetName()));
		}
		else
		{
		Context.AddHighPriorityLine(FString::Printf(TEXT("auto %s = CastChecked<%s>(GetDefaultSubobjectByName(TEXT(\"%s\")));")
			, *LocalNativeName, *FEmitHelper::GetCppName(ObjectClass), *Object->GetName()));
		}

		const UObject* ObjectArchetype = Object->GetArchetype();
		for (auto Property : TFieldRange<const UProperty>(ObjectClass))
		{
			OuterGenerate(Context, Property, LocalNativeName
				, reinterpret_cast<const uint8*>(Object)
				, reinterpret_cast<const uint8*>(ObjectArchetype)
				, EPropertyAccessOperator::Pointer);
		}
	}
	else
	{
		const FString OuterStr = Context.FindGloballyMappedObject(Object);
		if (OuterStr.IsEmpty())
		{
			ensure(false);
			return FString();
		}
		Context.AddHighPriorityLine(FString::Printf(TEXT("auto %s = NewObject<%s>(%s, TEXT(\"%s\"));")
			, *LocalNativeName
			, *FEmitHelper::GetCppName(ObjectClass)
			, *OuterStr
			, *Object->GetName()));
	}

	return LocalNativeName;
}