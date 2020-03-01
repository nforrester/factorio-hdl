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

; The first list has to run out first
(define zip
  (lambda lists
    (if (equal? (car lists) ())
      ()
      (cons (map car lists) (apply zip (map cdr lists))))))

(define take
  (lambda (n xs)
    (if (eq? xs ())
      ()
      (if (eqv? 0 n)
        ()
        (cons (car xs) (take (- n 1) (cdr xs)))))))

(define drop
  (lambda (n xs)
    (if (eq? xs ())
      ()
      (if (eqv? 0 n)
        xs
        (drop (- n 1) (cdr xs))))))

(define chunks-of
  (lambda (n xs)
    (if (eq? xs ())
      ()
      (cons (take n xs) (chunks-of n (drop n xs))))))

(define strings->symbol
  (lambda strings
    (string->symbol (apply string-append strings))))

(define list-of-n-symbols
  (lambda (base-name n)
    (for (range n) (lambda (i)
        (strings->symbol (symbol->string base-name) (number->string i))))))

; Like lambda, but destructures a list into the argument list.
(defmacro lambda+ (args . body)
  (let ((args-sym (gentemp)))
    `(lambda (,args-sym)
       ((flip apply) ,args-sym
         (lambda ,args ,@body)))))

(define assert
  (lambda (bool message)
    (if (not bool)
      (raise (error message)))))

(define lookup
  (lambda (alist key)
    (letrec ((loop (lambda (xs)
                     (if (eqv? (caar xs) key)
                       (cadar xs)
                       (loop (cdr xs))))))
      (loop alist))))

(define remove-first-equal
  (lambda (x elems)
    (letrec ((loop (lambda (result xs found)
                     (if (eq? xs ())
                       result
                       (if (or found (not (equal? x (car xs))))
                         (loop (cons (car xs) result) (cdr xs) found)
                         (loop result (cdr xs) #t))))))
      (reverse (loop () elems #f)))))

; insertion sort - O(n^2)
(define sort-by-key
  (lambda (key-fun elems)
    (letrec ((find-max (lambda (max-key max-val xs)
                         (if (eq? xs ())
                           max-val
                           (let* ((val (car xs))
                                  (key (key-fun val)))
                             (if (> key max-key)
                               (find-max key val (cdr xs))
                               (find-max max-key max-val (cdr xs)))))))
             (loop (lambda (sorted unsorted)
                     (if (eq? unsorted ())
                       sorted
                       (let ((max-val (find-max (key-fun (car unsorted))
                                                (car unsorted)
                                                (cdr unsorted))))
                         (loop (cons max-val sorted)
                               (remove-first-equal max-val unsorted)))))))
      (loop () elems))))

(define replicate
  (lambda (n x)
    (letrec ((loop (lambda (result remaining)
                     (if (eqv? remaining 0)
                       result
                       (loop (cons x result) (- remaining 1))))))
      (loop () n))))

(define to-hex
  (lambda (n)
    (letrec ((loop (lambda (hex remaining)
                     (let* ((low-nibble (logand #xf remaining))
                            (next-digit (case low-nibble
                                          ((#x0) #\0)
                                          ((#x1) #\1)
                                          ((#x2) #\2)
                                          ((#x3) #\3)
                                          ((#x4) #\4)
                                          ((#x5) #\5)
                                          ((#x6) #\6)
                                          ((#x7) #\7)
                                          ((#x8) #\8)
                                          ((#x9) #\9)
                                          ((#xa) #\a)
                                          ((#xb) #\b)
                                          ((#xc) #\c)
                                          ((#xd) #\d)
                                          ((#xe) #\e)
                                          ((#xf) #\f))))
                       (if (eqv? remaining 0)
                         (list->string hex)
                         (loop (cons next-digit hex) (arithmetic-shift remaining -4)))))))
      (loop () n))))
