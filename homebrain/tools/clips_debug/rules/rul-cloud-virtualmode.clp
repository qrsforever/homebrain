(defrule rul-cloud-virtualmode
    ?f <- (virtualmode ?value)
  =>
    (bind ?c (create-rule-context rul-cloud-virtualmode))
    (logi "virtualmode " ?value)
    (switch ?value
     (case 0
      then
         (assert (switch-scene room0 none))
     )
     (case 1
      then
         (assert (switch-scene room1 getup))
     )
     (case 2
      then
         (assert (switch-scene room1 sleep))
     )
     (case 3
      then
         (assert (switch-scene room0 wateroverflow))
     )
     (case 4
      then
         (assert (switch-scene room0 smokealarm))
     )
     (case 5
      then
         (assert (switch-scene room0 gasalarm))
     )
     (case 6
      then
         (assert (switch-scene room0 sosalarm))
     )
     (case 7
      then
         (assert (switch-scene room0 infraredalarm))
     )
     (case 8
      then
         (assert (switch-scene room1 entermovie))
     )
     (case 9
      then
         (assert (switch-scene room1 comehome))
     )
     (case 10
      then
         (assert (switch-scene room1 leavehome))
     )
     (default (logw "unkown mode " ?value))
    )
)
