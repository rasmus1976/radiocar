* How long time did you end up spending on this coding test?

I spent around 5 hours in total.
	- About 30 mins preparing my (very old) private laptop with compilers, git etc.
	- ~2 hours reminding myself about windows console API:s etc, been a while since I wrote console apps...
	- Remaing ~2,5 hours for actual implementation.

* Explain why you chose the code structure(s) you used in your solution.

I most often work with C in embedded/RTOS environments. This task is thus a bit "off" for me.
Also, realized at the very end that C wasn't allowed (but Javascript was!? hmm...).
Checked with hiring manager and he said it was ok to proceed with my initial C solution.
I simply do not have time to restart from scratch.
Also, once reading your description a bit more carefully, I'm not sure about if the sim is supposed to be in runtime or not.
Mine is. Successful "sim" is not to hit wall within approx. 60 seconds.  

* What would you add to your solution if you had more time? This question is especially important if you did not spend much time on the coding test - use this as an opportunity to explain what your solution is missing.

There is a lot that would need refinement and enhancement. 
Also threading it really not necassary but could in the end make main() very "clean".
I would probably add proper event-handler with callbacks etc (instead of just a mutex protected variable, not "pretty"...)
And a more correct way to handle start direction vs. movement... this is just a "quick fix" to get car initally moving at all.

* What did you think of this recruitment test?

I don't think it is an appropiate test/task for embedded developers, I havn't done stuff like this since LTH about 20 years ago ;)
It looks more suitable for a "fullstack developer" who is focusing on backend/server-side within for example MS .Net or similar.
However a little bit funny to re-visit win console API:s ;)
