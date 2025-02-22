#include <iostream>
#include "../boundary.h"
#include "shared/shared.h"
#include "shared/string.h"
#include "../../region/common/controlblock.h"
#include "../../region/common/heap.h"
#include "../../region/linear/linear.h"

#include "../../translatetype.h"

#include "../expression.h"

Ref buildExternCall(
    GlobalState* globalState,
    FunctionState* functionState,
    LLVMBuilderRef builder,
    Prototype* prototype,
    const std::vector<Ref>& args) {
  if (prototype->name->name == "__vbi_addI32") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildAdd(builder, leftLE, rightLE,"add");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_multiplyI32") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto resultIntLE = LLVMBuildMul(builder, leftLE, rightLE, "mul");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, resultIntLE);
  } else if (prototype->name->name == "__vbi_subtractI32") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto resultIntLE = LLVMBuildSub(builder, leftLE, rightLE, "diff");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, resultIntLE);
  } else if (prototype->name->name == "__vbi_lessThanI32") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildICmp(builder, LLVMIntSLT, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_greaterThanI32") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildICmp(builder, LLVMIntSGT, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_lessThanFloat") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildFCmp(builder, LLVMRealOLT, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_greaterThanFloat") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildFCmp(builder, LLVMRealOGT, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_greaterThanOrEqI32") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildICmp(builder, LLVMIntSGE, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_lessThanOrEqI32") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildICmp(builder, LLVMIntSLE, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_eqI32") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildICmp(builder, LLVMIntEQ, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_modI32") {
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    assert(args.size() == 2);
    auto result = LLVMBuildSRem( builder, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_divideI32") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildSDiv(builder, leftLE, rightLE,"add");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_addI64") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildAdd(builder, leftLE, rightLE,"add");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_multiplyI64") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto resultIntLE = LLVMBuildMul(builder, leftLE, rightLE, "mul");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, resultIntLE);
  } else if (prototype->name->name == "__vbi_subtractI64") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto resultIntLE = LLVMBuildSub(builder, leftLE, rightLE, "diff");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, resultIntLE);
  } else if (prototype->name->name == "__vbi_lessThanI64") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildICmp(builder, LLVMIntSLT, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_greaterThanI64") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildICmp(builder, LLVMIntSGT, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_greaterThanOrEqI64") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildICmp(builder, LLVMIntSGE, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_lessThanOrEqI64") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildICmp(builder, LLVMIntSLE, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_eqI64") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildICmp(builder, LLVMIntEQ, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_modI64") {
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    assert(args.size() == 2);
    auto result = LLVMBuildSRem( builder, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_divideI64") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildSDiv(builder, leftLE, rightLE,"add");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_divideFloatFloat") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildFDiv(builder, leftLE, rightLE,"divided");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_multiplyFloatFloat") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildFMul(builder, leftLE, rightLE,"multiplied");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_subtractFloatFloat") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildFSub(builder, leftLE, rightLE,"subtracted");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_negateFloat") {
    assert(args.size() == 1);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto result = LLVMBuildFNeg(builder, leftLE, "negated");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_strLength") {
    assert(args.size() == 1);
    auto resultLenLE = globalState->getRegion(globalState->metalCache->strRef)->getStringLen(functionState, builder, args[0]);
    globalState->getRegion(globalState->metalCache->strRef)
        ->dealias(FL(), functionState, builder, globalState->metalCache->strRef, args[0]);
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, resultLenLE);
  } else if (prototype->name->name == "__vbi_addFloatFloat") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildFAdd(builder, leftLE, rightLE, "add");
    return wrap(globalState->getRegion(globalState->metalCache->floatRef), globalState->metalCache->floatRef, result);
  } else if (prototype->name->name == "__vbi_panic") {
    buildPrint(globalState, builder, "(panic)\n");
    // See MPESC for status codes
    auto exitCodeLE = makeConstIntExpr(functionState, builder, LLVMInt64TypeInContext(globalState->context), 1);
    LLVMBuildCall(builder, globalState->externs->exit, &exitCodeLE, 1, "");
    LLVMBuildRet(builder, LLVMGetUndef(functionState->returnTypeL));
    return wrap(globalState->getRegion(globalState->metalCache->neverRef), globalState->metalCache->neverRef, globalState->neverPtr);
  } else if (prototype->name->name == "__vbi_getch") {
    auto resultIntLE = LLVMBuildCall(builder, globalState->externs->getch, nullptr, 0, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, resultIntLE);
  } else if (prototype->name->name == "__vbi_eqFloatFloat") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildFCmp(builder, LLVMRealOEQ, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_eqBoolBool") {
    assert(args.size() == 2);
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    auto result = LLVMBuildICmp(builder, LLVMIntEQ, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_not") {
    assert(args.size() == 1);
    auto result = LLVMBuildNot(
        builder,
        checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]),
        "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_and") {
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    assert(args.size() == 2);
    auto result = LLVMBuildAnd( builder, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else if (prototype->name->name == "__vbi_or") {
    auto leftLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[0], args[0]);
    auto rightLE = checkValidInternalReference(FL(), globalState, functionState, builder, prototype->params[1], args[1]);
    assert(args.size() == 2);
    auto result = LLVMBuildOr( builder, leftLE, rightLE, "");
    return wrap(globalState->getRegion(prototype->returnType), prototype->returnType, result);
  } else {
    auto valeArgRefs = std::vector<Ref>{};
    valeArgRefs.reserve(args.size());
    for (int i = 0; i < args.size(); i++) {
      valeArgRefs.push_back(args[i]);
    }

    auto hostArgsLE = std::vector<LLVMValueRef>{};
    hostArgsLE.reserve(args.size() + 1);

    auto sizeArgsLE = std::vector<LLVMValueRef>{};
    sizeArgsLE.reserve(args.size() + 1);

    for (int i = 0; i < args.size(); i++) {
      auto valeArgRefMT = prototype->params[i];
      auto hostArgRefMT =
          (valeArgRefMT->ownership == Ownership::SHARE ?
              globalState->linearRegion->linearizeReference(valeArgRefMT) :
              valeArgRefMT);

      auto valeArg = valeArgRefs[i];
      auto [hostArgRefLE, argSizeLE] =
          sendValeObjectIntoHost(
              globalState, functionState, builder, valeArgRefMT, hostArgRefMT, valeArg);
      if (typeNeedsPointerParameter(globalState, valeArgRefMT)) {
        auto hostArgRefLT = globalState->getRegion(valeArgRefMT)->getExternalType(valeArgRefMT);
        assert(LLVMGetTypeKind(hostArgRefLT) != LLVMPointerTypeKind);
        hostArgsLE.push_back(makeBackendLocal(functionState, builder, hostArgRefLT, "ptrParamLocal", hostArgRefLE));
      } else {
        hostArgsLE.push_back(hostArgRefLE);
      }
      if (includeSizeParam(globalState, prototype, i)) {
        sizeArgsLE.push_back(argSizeLE);
      }
    }

    hostArgsLE.insert(hostArgsLE.end(), sizeArgsLE.begin(), sizeArgsLE.end());
    sizeArgsLE.clear();

    auto externFuncIter = globalState->externFunctions.find(prototype->name->name);
    assert(externFuncIter != globalState->externFunctions.end());
    auto externFuncL = externFuncIter->second;

    buildFlare(FL(), globalState, functionState, builder, "Suspending function ", functionState->containingFuncName);
    buildFlare(FL(), globalState, functionState, builder, "Calling extern function ", prototype->name->name);

    LLVMValueRef hostReturnLE = nullptr;
    if (typeNeedsPointerParameter(globalState, prototype->returnType)) {
      auto hostReturnRefLT = globalState->getRegion(prototype->returnType)->getExternalType(prototype->returnType);
      auto localPtrLE =
          makeBackendLocal(functionState, builder, hostReturnRefLT, "retOutParam", LLVMGetUndef(hostReturnRefLT));
      buildFlare(FL(), globalState, functionState, builder, "Return ptr! ", ptrToIntLE(globalState, builder, localPtrLE));
      hostArgsLE.insert(hostArgsLE.begin(), localPtrLE);
      LLVMBuildCall(builder, externFuncL, hostArgsLE.data(), hostArgsLE.size(), "");
      hostReturnLE = LLVMBuildLoad(builder, localPtrLE, "hostReturn");
      buildFlare(FL(), globalState, functionState, builder, "Loaded the return! ", LLVMABISizeOfType(globalState->dataLayout, LLVMTypeOf(hostReturnLE)));
    } else {
      hostReturnLE = LLVMBuildCall(builder, externFuncL, hostArgsLE.data(), hostArgsLE.size(), "");
    }

    buildFlare(FL(), globalState, functionState, builder, "Done calling function ", prototype->name->name);
    buildFlare(FL(), globalState, functionState, builder, "Resuming function ", functionState->containingFuncName);

    if (prototype->returnType->kind == globalState->metalCache->never) {
      LLVMBuildRet(builder, LLVMGetUndef(functionState->returnTypeL));
      return wrap(globalState->getRegion(globalState->metalCache->neverRef), globalState->metalCache->neverRef, globalState->neverPtr);
    } else {
      if (prototype->returnType == globalState->metalCache->voidRef) {
        return makeVoidRef(globalState);
      } else {
        buildFlare(FL(), globalState, functionState, builder);

        auto valeReturnRefMT = prototype->returnType;
        auto hostReturnMT =
            (valeReturnRefMT->ownership == Ownership::SHARE ?
                globalState->linearRegion->linearizeReference(valeReturnRefMT) :
                valeReturnRefMT);

        auto valeReturnRef =
            receiveHostObjectIntoVale(
                globalState, functionState, builder, hostReturnMT, valeReturnRefMT, hostReturnLE);

        return valeReturnRef;
      }
    }
  }
  assert(false);
}

Ref translateExternCall(
    GlobalState* globalState,
    FunctionState* functionState,
    BlockState* blockState,
    LLVMBuilderRef builder,
    ExternCall* call) {
  auto name = call->function->name->name;
  auto params = call->function->params;
  std::vector<Ref> args;
  assert(call->argExprs.size() == call->argTypes.size());
  for (int i = 0; i < call->argExprs.size(); i++) {
    args.emplace_back(
        translateExpression(globalState, functionState, blockState, builder, call->argExprs[i]));
  }
  return buildExternCall(globalState, functionState, builder, call->function, args);
}
