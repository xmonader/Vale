#include "function/function.h"
#include "function/expressions/expressions.h"
#include "globalstate.h"

std::tuple<LLVMValueRef, LLVMBuilderRef> makeStringSetupFunction(GlobalState* globalState) {
  auto voidLT = LLVMVoidTypeInContext(globalState->context);

  auto functionLT = LLVMFunctionType(voidLT, nullptr, 0, false);
  auto functionL = LLVMAddFunction(globalState->mod, "__Vale_SetupStrings", functionLT);

  auto stringsBuilder = LLVMCreateBuilderInContext(globalState->context);
  LLVMBasicBlockRef blockL = LLVMAppendBasicBlockInContext(globalState->context, functionL, "stringsBlock");
  LLVMPositionBuilderAtEnd(stringsBuilder, blockL);
  auto ret = LLVMBuildRetVoid(stringsBuilder);
  LLVMPositionBuilderBefore(stringsBuilder, ret);

  return {functionL, stringsBuilder};
}


Prototype* makeValeMainFunction(
    GlobalState* globalState,
    LLVMValueRef stringSetupFunctionL,
    Prototype* mainSetupFuncProto,
    Prototype* userMainFunctionPrototype,
    Prototype* mainCleanupFunctionPrototype) {
  auto voidLT = LLVMVoidTypeInContext(globalState->context);
  auto int1LT = LLVMInt1TypeInContext(globalState->context);
  auto int8LT = LLVMInt8TypeInContext(globalState->context);
  auto int32LT = LLVMInt32TypeInContext(globalState->context);
  auto int32PtrLT = LLVMPointerType(int32LT, 0);
  auto int64LT = LLVMInt64TypeInContext(globalState->context);
  auto voidPtrLT = LLVMPointerType(int8LT, 0);
  auto int8PtrLT = LLVMPointerType(int8LT, 0);

  auto valeMainName = globalState->metalCache->getName(globalState->metalCache->builtinPackageCoord, "__Vale_Main");
  auto valeMainProto =
      globalState->metalCache->getPrototype(valeMainName, globalState->metalCache->i64Ref, {});
  declareAndDefineExtraFunction(
      globalState, valeMainProto, valeMainName->name,
      [globalState, stringSetupFunctionL, mainSetupFuncProto, userMainFunctionPrototype, mainCleanupFunctionPrototype](
          FunctionState *functionState, LLVMBuilderRef entryBuilder) {
        buildFlare(FL(), globalState, functionState, entryBuilder);

        LLVMBuildCall(entryBuilder, stringSetupFunctionL, nullptr, 0, "");
        LLVMBuildCall(entryBuilder, globalState->lookupFunction(mainSetupFuncProto), nullptr, 0, "");
//
//        LLVMBuildStore(
//            entryBuilder,
//            LLVMBuildUDiv(
//                entryBuilder,
//                LLVMBuildPointerCast(
//                    entryBuilder,
//                    globalState->writeOnlyGlobal,
//                    LLVMInt64TypeInContext(globalState->context),
//                    "ptrAsIntToWriteOnlyGlobal"),
//                constI64LE(globalState, 8),
//                "ram64IndexToWriteOnlyGlobal"),
//            globalState->ram64IndexToWriteOnlyGlobal);

        buildFlare(FL(), globalState, functionState, entryBuilder);
        if (globalState->opt->census) {
          // Add all the edges to the census, so we can check that fat pointers are right.
          // We remove them again at the end of outer main.
          // We should one day do this for all globals.
          for (auto edgeAndItablePtr : globalState->interfaceTablePtrs) {
            auto itablePtrLE = edgeAndItablePtr.second;
            LLVMValueRef itablePtrAsVoidPtrLE =
                LLVMBuildBitCast(
                    entryBuilder, itablePtrLE, LLVMPointerType(LLVMInt8TypeInContext(globalState->context), 0), "");

            //buildFlare(FL(), globalState, functionState, entryBuilder, ptrToIntLE(globalState, entryBuilder, itablePtrAsVoidPtrLE));
            LLVMBuildCall(entryBuilder, globalState->externs->censusAdd, &itablePtrAsVoidPtrLE, 1, "");
          }
          buildFlare(FL(), globalState, functionState, entryBuilder);
        }
        buildFlare(FL(), globalState, functionState, entryBuilder);

        auto userMainResultRef = buildCall(globalState, functionState, entryBuilder, userMainFunctionPrototype, {});
        auto userMainResultLE =
            globalState->getRegion(userMainFunctionPrototype->returnType)
                ->checkValidReference(
                    FL(), functionState, entryBuilder, userMainFunctionPrototype->returnType, userMainResultRef);

        buildFlare(FL(), globalState, functionState, entryBuilder);
        buildCall(globalState, functionState, entryBuilder, mainCleanupFunctionPrototype, {});
        buildFlare(FL(), globalState, functionState, entryBuilder);

        if (globalState->opt->printMemOverhead) {
          buildFlare(FL(), globalState, functionState, entryBuilder);
          buildPrint(globalState, entryBuilder, "\nLiveness checks: ");
          buildPrint(
              globalState, entryBuilder,
              LLVMBuildLoad(entryBuilder, globalState->livenessCheckCounter, "livenessCheckCounter"));
          buildPrint(globalState, entryBuilder, "\n");
        }
        buildFlare(FL(), globalState, functionState, entryBuilder);

        if (globalState->opt->census) {
          buildFlare(FL(), globalState, functionState, entryBuilder);
          // Remove all the things from the census that we added at the start of the program.
          for (auto edgeAndItablePtr : globalState->interfaceTablePtrs) {
            auto itablePtrLE = edgeAndItablePtr.second;
            LLVMValueRef itablePtrAsVoidPtrLE =
                LLVMBuildBitCast(
                    entryBuilder, itablePtrLE, LLVMPointerType(LLVMInt8TypeInContext(globalState->context), 0), "");
            LLVMBuildCall(entryBuilder, globalState->externs->censusRemove, &itablePtrAsVoidPtrLE, 1, "");
          }
          buildFlare(FL(), globalState, functionState, entryBuilder);

          LLVMValueRef numLiveObjAssertArgs[3] = {
              LLVMConstInt(LLVMInt64TypeInContext(globalState->context), 0, false),
              LLVMBuildLoad(entryBuilder, globalState->liveHeapObjCounter, "numLiveObjs"),
              globalState->getOrMakeStringConstant("Memory leaks!"),
          };
          LLVMBuildCall(entryBuilder, globalState->externs->assertI64Eq, numLiveObjAssertArgs, 3, "");
        }
        buildFlare(FL(), globalState, functionState, entryBuilder);

        if (userMainFunctionPrototype->returnType->kind == globalState->metalCache->vooid) {
          buildFlare(FL(), globalState, functionState, entryBuilder);
          LLVMBuildRet(entryBuilder, LLVMConstInt(LLVMInt64TypeInContext(globalState->context), 0, true));
        } else if (userMainFunctionPrototype->returnType->kind == globalState->metalCache->i64) {
          buildFlare(FL(), globalState, functionState, entryBuilder, userMainResultLE);
          LLVMBuildRet(entryBuilder, userMainResultLE);
        } else if (userMainFunctionPrototype->returnType->kind == globalState->metalCache->i32) {
          buildFlare(FL(), globalState, functionState, entryBuilder, userMainResultLE);
          LLVMBuildRet(entryBuilder, LLVMBuildZExt(entryBuilder, userMainResultLE, LLVMInt64TypeInContext(globalState->context), "extended"));
        } else if (userMainFunctionPrototype->returnType->kind == globalState->metalCache->never) {
          buildFlare(FL(), globalState, functionState, entryBuilder);
          LLVMBuildRet(entryBuilder, LLVMConstInt(LLVMInt64TypeInContext(globalState->context), 0, true));
        } else {
          assert(false);
        }

        return userMainResultLE;
      });

  return valeMainProto;
}

LLVMValueRef makeEntryFunction(
    GlobalState* globalState,
    Prototype* valeMainPrototype) {
  auto voidLT = LLVMVoidTypeInContext(globalState->context);
  auto int1LT = LLVMInt1TypeInContext(globalState->context);
  auto int8LT = LLVMInt8TypeInContext(globalState->context);
  auto int32LT = LLVMInt32TypeInContext(globalState->context);
  auto int32PtrLT = LLVMPointerType(int32LT, 0);
  auto int64LT = LLVMInt64TypeInContext(globalState->context);
  auto voidPtrLT = LLVMPointerType(int8LT, 0);
  auto int8PtrLT = LLVMPointerType(int8LT, 0);

  // This is the actual entry point for the binary. However, it wont contain much.
  // It'll just have a
  // This will be populated at the end, we're just making it here so we can call it
  auto entryParamsLT = std::vector<LLVMTypeRef>{ int64LT, LLVMPointerType(LLVMPointerType(int8LT, 0), 0) };
  LLVMTypeRef functionTypeL = LLVMFunctionType(int64LT, entryParamsLT.data(), entryParamsLT.size(), 0);
  LLVMValueRef entryFunctionL = LLVMAddFunction(globalState->mod, "main", functionTypeL);

  LLVMSetLinkage(entryFunctionL, LLVMDLLExportLinkage);
  LLVMSetDLLStorageClass(entryFunctionL, LLVMDLLExportStorageClass);
  LLVMSetFunctionCallConv(entryFunctionL, LLVMX86StdcallCallConv);
  LLVMBuilderRef entryBuilder = LLVMCreateBuilderInContext(globalState->context);
  LLVMBasicBlockRef blockL =
      LLVMAppendBasicBlockInContext(globalState->context, entryFunctionL, "thebestblock");
  LLVMPositionBuilderAtEnd(entryBuilder, blockL);


  auto numArgsLE = LLVMGetParam(entryFunctionL, 0);
  auto argsLE = LLVMGetParam(entryFunctionL, 1);
  LLVMBuildStore(entryBuilder, numArgsLE, globalState->numMainArgs);
  LLVMBuildStore(entryBuilder, argsLE, globalState->mainArgs);

  auto mainResultLE = LLVMBuildCall(entryBuilder, globalState->lookupFunction(valeMainPrototype), nullptr, 0, "");

  LLVMBuildRet(entryBuilder, mainResultLE);
  LLVMDisposeBuilder(entryBuilder);

  return entryFunctionL;
}