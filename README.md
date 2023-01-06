# A Krivine Machine for Open Terms
A very basic, and far from optimal, reference implementation of a Krivine machine for open terms, as described by García-Pérez, Á., Nogueira, P., & Moreno-Navarro, J. J.

#### NOT Implemented
- Error Handling
- Garbage Collection
- Names/Strings/Atoms
- Saved Expressions/Environments

## References

##### Introduces the Krivine (K) machine for, big step, weak head normalization, of lambda closure calculus, with closed terms.
> Krivine, J. L. (1985). Un interpréteur du lambda-calcul. Available on http://www.pps.jussieu.fr/~krivine/articles.

##### Introduces the KN machine for, big step, full normalization, of lambda closure calculus, with closed terms.
> Crégut, P. (1990, May). An abstract machine for lambda-terms normalization. In Proceedings of the 1990 ACM conference on LISP and functional programming (pp. 333-340).

##### Introduces an improved KN machine for, big step, full normalization, of lambda closure calculus, with open terms.
> García-Pérez, Á., Nogueira, P., & Moreno-Navarro, J. J. (2013, September). Deriving the full-reducing Krivine machine from the small-step operational semantics of normal order. In Proceedings of the 15th Symposium on Principles and Practice of Declarative Programming (pp. 85-96).

##### Additional reading for the improved KN machine.
> Garcia-Perez, A., & Nogueira, P. (2019). The full-reducing Krivine abstract machine KN simulates pure normal-order reduction in lockstep: A proof via corresponding calculus. Journal of Functional Programming, 29.

##### Álvaro García Perez presents his above 2019 work at ICFP 2020. (YouTube)
> [The full-reducing Krivine abstract machine KN simulates pure normal-order reduction i... (ICFP 2020)](https://youtu.be/jRjxCd0aoMk)

##### Source (possibly not original) of the square bracket de Bruijn notation I&apos;m using.
> [The Programmer&apos;s Ring](https://loup-vaillant.fr/articles/programming-ring)
