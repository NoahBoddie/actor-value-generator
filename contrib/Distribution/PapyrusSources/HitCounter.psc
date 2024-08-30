scriptName HitCounter hidden

Bool function StartCounting(Actor target) global native

Bool function StopCounting(Actor target) global native

Int function GetTotalHitCounters() global native

function Increment(Actor target, Int by = 1) global native

Int function GetCount(Actor target) global native
