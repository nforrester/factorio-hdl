(define test-macro
  (lambda (some-argument)
    `(hello ,some-argument worlds)))

(define test-macro2
  (lambda ()
    `(begin (stuff) (more-stuff))))
