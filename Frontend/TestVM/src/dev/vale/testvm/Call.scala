package dev.vale.testvm

import dev.vale.finalast.{KindH, ReferenceH}
import dev.vale.{vassert, vassertSome, vfail}
import dev.vale.finalast.{KindH, ReferenceH}
import dev.vale.vfail

import scala.collection.mutable

class Call(callId: CallId, in_args: Vector[ReferenceV]) {
  private val args = mutable.HashMap[Int, Option[ReferenceV]]() ++ in_args.indices.zip(in_args.map(arg => Some(arg))).toMap

  private val locals = mutable.HashMap[VariableAddressV, VariableV]()

  def addLocal(varAddr: VariableAddressV, reference: ReferenceV, tyype: ReferenceH[KindH]): Unit = {
    vassert(varAddr.callId == callId)
    vassert(!locals.contains(varAddr))
    vassert(!locals.exists(_._1.local.id.number == varAddr.local.id.number))
    locals.put(varAddr, VariableV(varAddr, reference, tyype))
  }

  def removeLocal(varAddr: VariableAddressV): Unit = {
    vassert(varAddr.callId == callId)
    vassert(locals.contains(varAddr))
    locals.remove(varAddr)
  }

  def getLocal(addr: VariableAddressV) = {
    vassertSome(locals.get(addr))
  }

  def mutateLocal(varAddr: VariableAddressV, reference: ReferenceV, expectedType: ReferenceH[KindH]): Unit = {
    locals(varAddr).reference = reference
  }

  def takeArgument(index: Int): ReferenceV = {
    vassert(index < args.size)
    args(index) match {
      case Some(ref) => {
        args.put(index, None)
        ref
      }
      case None => {
        vfail("Already took from argument " + index)
      }
    }
  }

  def prepareToDie() = {
    vassert(locals.isEmpty)

//    // Make sure all locals were unletted
//    locals.foreach({ case (varAddr, variable) =>
//      // We trip this when we don't Unstackify something so its still alive on
//      // the stack.
//      vassert(variable.reference == None)
//      locals.remove(varAddr)
//    })
//    while (localAddrStack.nonEmpty) {
//      localAddrStack.pop()
//    }
//
//    vassert(localAddrStack.size == locals.size)

    val undeadArgs =
      args.collect({
        case (index, Some(value)) => (index, value)
      })
    if (undeadArgs.nonEmpty) {
      vfail("Undead arguments:\n" + undeadArgs.mkString("\n"))
    }

//    val undeadRegisters =
//      registersById.collect({
//        case (registerId, Some(register)) => (registerId, register)
//      })
//    if (undeadRegisters.nonEmpty) {
//      vfail("Undead registers:\n" + undeadRegisters.mkString("\n"))
//    }
  }
}
