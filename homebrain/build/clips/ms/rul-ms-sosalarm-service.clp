;=================================================================
; date: 2019-07-05 21:57
; title: sosalarm micro service
; author: QRS
;=================================================================

(defrule rul-ms-sosalarm-service
    (datetime ?clock $?ox)
    ?svr <- (service
        (name ?name) (skey ?key &:(eq ?key ?name))
        (state ?st) (saves $?saves)
        (args ?sos ?alarm ?light ?timelen $?oy)
        (ntime ?tm &:(or (> ?clock ?tm) (neq ?st running)))
    )
    ; (object (is-a oic.d.sosalarm) (ID ?sos)))
    (object (is-a oic.d.alarmer) (ID ?alarm))
    (object (is-a oic.d.light) (ID ?light) (brightness ?brg ?brg-old))
  =>
    (logi "service: " ?name ", skey: " ?key ", state: " ?st)
    (if (eq ?st stop)
     then
        (assert (switch-scene default none))
        (return)
    )
    (if (eq ?st start)
     then
        (act-control ?alarm alarm 1)
        (bind ?etime (+ ?timelen (nth$ 1 (now))))
        (modify ?svr (state running) (saves ?etime))
        (return)
    )
    (if (= ?brg 10)
     then
        (act-control ?light brightness 100)
     else
        (act-control ?light brightness 10)
    )
    (if (< ?clock (nth$ 1 ?saves))
     then
        (modify ?svr (ntime (+ ?clock 500)))
     else
        ; (act-control ?sos value 0)
        (act-control ?alarm alarm 0)
        (act-control ?light value 1)
        (assert (stop-service ?name))
    )
)
