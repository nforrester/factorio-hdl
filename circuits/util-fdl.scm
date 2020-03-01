(load "circuits/util.scm")

(define defpart-constant-all-ones
  (lambda ()
    `(defpart constant-all-ones
       ((out yellow o))
       ,@(for (chunks-of max-signals-per-constant-combinator all-signals)
           (lambda (chunk-of-sigs)
             `(constant o ,(for chunk-of-sigs (lambda (sig) `(,sig 1)))))))))
