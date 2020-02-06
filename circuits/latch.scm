(load "circuits/util.scm")

(define defpart-constant-all-ones
  (lambda ()
    `(defpart constant-all-ones
       ((out yellow o))
       (constant o ,(for all-signals (lambda (sig) `(,sig 1)))))))
