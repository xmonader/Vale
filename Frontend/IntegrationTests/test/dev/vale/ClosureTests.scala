package dev.vale

import dev.vale.postparsing.{CodeVarNameS, IVariableUseCertainty, LocalS, NotUsed, Used}
import dev.vale.typing.ast.{AddressMemberLookupTE, ConstructTE, FunctionCallTE, LetNormalTE, LocalLookupTE, MutateTE, ReferenceMemberLookupTE}
import dev.vale.typing.env.{AddressibleLocalVariableT, ReferenceLocalVariableT}
import dev.vale.typing.expression.LocalHelper
import dev.vale.typing.names.{ClosureParamNameT, CodeVarNameT, FullNameT, FunctionNameT, LambdaCitizenNameT, TypingPassBlockResultVarNameT}
import dev.vale.typing.types.{AddressMemberTypeT, CoordT, FinalT, ImmutableT, IntT, MutableT, ReferenceMemberTypeT, ShareT, StructMemberT, StructTT, VaryingT}
import dev.vale.postparsing._
import dev.vale.typing._
import dev.vale.typing.ast._
import dev.vale.typing.env.ReferenceLocalVariableT
import dev.vale.typing.types._
import org.scalatest.{FunSuite, Matchers}
import dev.vale.typing.names.LambdaCitizenNameT
import dev.vale.von.VonInt

class ClosureTests extends FunSuite with Matchers {

  test("Addressibility") {
    def calc(
        selfBorrowed: IVariableUseCertainty,
        selfMoved: IVariableUseCertainty,
        selfMutated: IVariableUseCertainty,
        childBorrowed: IVariableUseCertainty,
        childMoved: IVariableUseCertainty,
        childMutated: IVariableUseCertainty) = {
      val addressibleIfMutable =
        LocalHelper.determineIfLocalIsAddressible(
          MutableT,
          LocalS(
            CodeVarNameS("x"), selfBorrowed, selfMoved, selfMutated, childBorrowed, childMoved, childMutated))
      val addressibleIfImmutable =
        LocalHelper.determineIfLocalIsAddressible(
          ImmutableT,
          LocalS(
            CodeVarNameS("x"), selfBorrowed, selfMoved, selfMutated, childBorrowed, childMoved, childMutated))
      (addressibleIfMutable, addressibleIfImmutable)
    }

    // If we don't do anything with the variable, it can be just a reference.
    calc(NotUsed, NotUsed, NotUsed, NotUsed, NotUsed, NotUsed) shouldEqual (false, false)

    // If we or our children only ever read, it can be just a reference.
    calc(Used, NotUsed, NotUsed,      NotUsed, NotUsed, NotUsed) shouldEqual (false, false)
    calc(NotUsed, NotUsed, NotUsed,   Used, NotUsed, NotUsed) shouldEqual (false, false)

    // If only we mutate it, it can be just a reference.
    calc(NotUsed, NotUsed, Used, NotUsed, NotUsed, NotUsed) shouldEqual (false, false)

    // Even if we're certain it's moved, it must be addressible.
    // Imagine:
    // exported func main() int {
    //   m = Marine();
    //   if (something) {
    //     something.consume(m);
    //   } else {
    //     otherthing.consume(m);
    //   }
    // }
    // (or, we can change it so we move it into the closure struct, but that
    // seems weird, i like thinking that closures only ever have borrows or
    // addressibles)
    // However, this doesnt apply to immutable, since move = copy.
    calc(NotUsed, NotUsed, NotUsed, NotUsed, Used, NotUsed) shouldEqual (true, false)

    // If we're certain children mutate it, it also has to be addressible.
    calc(NotUsed, NotUsed, NotUsed, NotUsed, NotUsed, Used) shouldEqual (true, true)
  }

  test("Captured own is borrow") {
    // Here, the scout determined that the closure is only ever borrowing
    // it (during the dereference to get its member) so typingpass doesn't put
    // an address into the closure, it instead puts a reference. Specifically,
    // a borrow reference (because why would we want to move this into the
    // closure struct?).
    // This means the closure struct contains a borrow reference. This means
    // the environment in the closure has to match this; the environment has
    // to have a borrow reference instead of an owning reference.

    val compile = RunCompilation.test(
      """
        |struct Marine {
        |  hp int;
        |}
        |exported func main() int {
        |  m = Marine(9);
        |  return { m.hp }();
        |}
      """.stripMargin)

    compile.evalForKind(Vector()) match { case VonInt(9) => }
  }

  test("Test closure's local variables") {
    val compile = RunCompilation.test("exported func main() int { x = 4; return {x}(); }")
    val coutputs = compile.expectCompilerOutputs()

    val main = coutputs.lookupLambdaIn("main")
    Collector.only(main, {
      case LetNormalTE(
        ReferenceLocalVariableT(
          FullNameT(_, Vector(FunctionNameT("main", _, _), LambdaCitizenNameT(_), FunctionNameT("__call", _, _)), ClosureParamNameT()),
          FinalT,
          CoordT(ShareT, StructTT(FullNameT(_, Vector(FunctionNameT("main", Vector(), Vector())), LambdaCitizenNameT(_))))),
        _) =>
    })
    Collector.only(main, {
      case LetNormalTE(
        ReferenceLocalVariableT(
          FullNameT(_, Vector(FunctionNameT("main",_,_), LambdaCitizenNameT(_), FunctionNameT("__call",_,_)),TypingPassBlockResultVarNameT(_)),
          FinalT,
          CoordT(ShareT,IntT.i32)),
        _) =>
    })
  }

  test("Test returning a nonmutable closured variable from the closure") {
    val compile = RunCompilation.test("exported func main() int { x = 4; return {x}(); }")
    val coutputs = compile.expectCompilerOutputs()
    val interner = compile.interner

    // The struct should have an int x in it which is a reference type.
    // It's a reference because we know for sure that it's moved from our child,
    // which means we don't need to check afterwards, which means it doesn't need
    // to be boxed/addressible.
    val closuredVarsStruct =
      vassertSome(
        coutputs.structs.find(struct => struct.fullName.last match { case l @ LambdaCitizenNameT(_) => true case _ => false }));
    val expectedMembers = Vector(StructMemberT(interner.intern(CodeVarNameT("x")), FinalT, ReferenceMemberTypeT(CoordT(ShareT, IntT.i32))));
    vassert(closuredVarsStruct.members == expectedMembers)

    val lambda = coutputs.lookupLambdaIn("main")
    // Make sure we're doing a referencememberlookup, since it's a reference member
    // in the closure struct.
    Collector.only(lambda, {
      case ReferenceMemberLookupTE(_,_, FullNameT(_, _, CodeVarNameT("x")), _, _) =>
    })

    // Make sure there's a function that takes in the closured vars struct, and returns an int
    val lambdaCall =
      vassertSome(
        coutputs.functions.find(func => {
          func.header.fullName.last match {
            case FunctionNameT("__call", _, _) => true
            case _ => false
          }
        }))
    lambdaCall.header.paramTypes.head match {
      case CoordT(ShareT, StructTT(FullNameT(_, Vector(FunctionNameT("main",Vector(),Vector())),LambdaCitizenNameT(_)))) =>
    }
    lambdaCall.header.returnType shouldEqual CoordT(ShareT, IntT.i32)

    // Make sure we make it with a function pointer and a constructed vars struct
    val main = coutputs.lookupFunction("main")
    Collector.only(main, {
      case ConstructTE(StructTT(FullNameT(_, Vector(FunctionNameT("main",Vector(),Vector())),LambdaCitizenNameT(_))), _, _) =>
    })

    // Make sure we call the function somewhere
    Collector.onlyOf(main, classOf[FunctionCallTE])

    Collector.only(lambda, {
      case LocalLookupTE(_,ReferenceLocalVariableT(FullNameT(_, _,ClosureParamNameT()),FinalT,_)) =>
    })

    compile.evalForKind(Vector()) match { case VonInt(4) => }
  }

  test("Mutates from inside a closure") {
    val compile = RunCompilation.test(
      """
        |exported func main() int {
        |  x = 4;
        |  { set x = x + 1; }();
        |  return x;
        |}
      """.stripMargin)
    val coutputs = compile.expectCompilerOutputs()
    val interner = compile.interner

    // The struct should have an int x in it.
    val closuredVarsStruct = vassertSome(coutputs.structs.find(struct => struct.fullName.last match { case l @ LambdaCitizenNameT(_) => true case _ => false }));
    val expectedMembers = Vector(StructMemberT(interner.intern(CodeVarNameT("x")), VaryingT, AddressMemberTypeT(CoordT(ShareT, IntT.i32))));
    vassert(closuredVarsStruct.members == expectedMembers)

    val lambda = coutputs.lookupLambdaIn("main")
    Collector.only(lambda, {
      case MutateTE(
        AddressMemberLookupTE(_,_,FullNameT(_, Vector(FunctionNameT("main",Vector(),Vector()), LambdaCitizenNameT(_)),CodeVarNameT("x")),CoordT(ShareT,IntT.i32), _),
        _) =>
    })

    val main = coutputs.lookupFunction("main")
    Collector.only(main, {
      case LetNormalTE(AddressibleLocalVariableT(_, VaryingT, _), _) =>
    })

    compile.evalForKind(Vector()) match { case VonInt(5) => }
  }

  test("Mutates from inside a closure inside a closure") {
    val compile = RunCompilation.test("exported func main() int { x = 4; { { set x = x + 1; }(); }(); return x; }")

    compile.evalForKind(Vector()) match { case VonInt(5) => }
  }

  test("Read from inside a closure inside a closure") {
    val compile = RunCompilation.test(
      """
        |exported func main() int {
        |  x = 42;
        |  return { { x }() }();
        |}
        |""".stripMargin)

    compile.evalForKind(Vector()) match { case VonInt(42) => }
  }

  test("Mutable lambda") {
    val compile =
      RunCompilation.test(
        Tests.loadExpected("programs/lambdas/lambdamut.vale"))

    val coutputs = compile.expectCompilerOutputs()
    val closureStruct =
      coutputs.structs.find(struct => {
        struct.fullName.last match {
          case LambdaCitizenNameT(_) => true
          case _ => false
        }
      }).get
    vassert(closureStruct.mutability == MutableT)
    compile.evalForKind(Vector()) match { case VonInt(42) => }
  }
}
