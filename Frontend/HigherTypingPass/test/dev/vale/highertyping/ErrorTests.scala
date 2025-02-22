package dev.vale.highertyping

import dev.vale.postparsing.CodeNameS
import dev.vale.{Err, Ok, vassert, vfail}
import dev.vale.postparsing.PostParser
import dev.vale.Err
import org.scalatest.{FunSuite, Matchers}

class ErrorTests extends FunSuite with Matchers  {
  def compileProgramForError(compilation: HigherTypingCompilation): ICompileErrorA = {
    compilation.getAstrouts() match {
      case Ok(result) => vfail("Expected error, but actually parsed invalid program:\n" + result)
      case Err(err) => err
    }
  }

  test("Report type not found") {
    val compilation =
      HigherTypingTestCompilation.test(
        """exported func main(a Bork) {
          |}
          |""".stripMargin)


    compileProgramForError(compilation) match {
      case e @ CouldntFindTypeA(_, CodeNameS("Bork")) => {
        val errorText = HigherTypingErrorHumanizer.humanize(compilation.getCodeMap().getOrDie(), e)
        vassert(errorText.contains("Couldn't find type `Bork`"))
      }
    }
  }

  test("Report couldnt solve rules") {
    val compilation =
      HigherTypingTestCompilation.test(
        """
          |func moo<A>(x int) {
          |  42
          |}
          |exported func main() {
          |  moo();
          |}
          |""".stripMargin)

    compileProgramForError(compilation) match {
      case e @ CouldntSolveRulesA(range, failure) => {
        val errorText = HigherTypingErrorHumanizer.humanize(compilation.getCodeMap().getOrDie(), e)
        vassert(errorText.contains("olve"))
      }
    }
  }
}
