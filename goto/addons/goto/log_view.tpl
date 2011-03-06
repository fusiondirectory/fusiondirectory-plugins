{if !$ACL}

	<h2>{msgPool type=permView}</h2>

{else}
	{if $logs_available}
	<h2>{t}Available logs{/t}</h2>

		<div style="width:99%;border: solid 1px #CCCCCC;">{$divlist}</div>
	  <br>
	  <p class="seperator"></p>
		<h2>{t}Selected log{/t}: {$selected_log}</h2>
		<div style="width:99%;height:350px;padding:3px;background-color:white; overflow-y: scroll;border: solid 1px;">
			{$log_file}
		</div>
	{else}
		<h2>{t}No logs for this host available!{/t}</h2>
	{/if}
{/if}

{if $standalone}
<br>
<input type="hidden" name="ignore" value="1">
<p class='seperator'></p>
<div style='text-align:right;width:99%; padding-right:5px; padding-top:5px;'>
	<input type='submit' name='abort_event_dialog' value='{msgPool type=backButton}'>
</div>
<br>
{/if}
