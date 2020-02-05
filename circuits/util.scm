(define nth
  (lambda (n xs)
    (if (eqv? 0 n)
      (car xs)
      (nth (- n 1) (cdr xs)))))

(define len
  (letrec ((help (lambda (n xs)
                    (if (equal? xs ())
                      n
                      (help (+ 1 n) (cdr xs))))))
    (lambda (xs) (help 0 xs))))

(define flip
  (lambda (f)
    (lambda (x y)
      (f y x))))

(define for (flip map))

(define range
  (lambda (n)
    (letrec ((f (lambda (i) (if (< i n) (cons i (f (+ 1 i))) ()))))
      (f 0))))

(define zip
  (lambda lists
    (if (equal? (car lists) ())
      ()
      (cons (map car lists) (apply zip (map cdr lists))))))

(define strings->symbol
  (lambda strings
    (string->symbol (apply string-append strings))))

(define list-of-n-symbols
  (lambda (base-name n)
    (for (range n) (lambda (i)
        (strings->symbol (symbol->string base-name) (number->string i))))))
