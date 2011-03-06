{if $init_failed}
	<h2>{t}Information{/t}</h2>
	<font style='color: #FF0000;'>{msgPool type=siError p=$message}</font>
	<input type='submit' name='reinit' value="{t}Retry{/t}">
{else}
	{if $type == 0}
		<h2><img class='center' alt='' src='plugins/opsi/images/hardware_info.png'>&nbsp;{t}Hardware information{/t}</h2>
	{else}
		<h2><img class='center' alt='' src='plugins/opsi/images/software_info.png'>&nbsp;{t}Software information{/t}</h2>
	{/if}

	{foreach from=$info item=item key=key}
	<div style='background-color: #E8E8E8; width: 100%; border: 2px dotted #CCCCCC;'>
	<h2>{t}Device{/t} {$key+1}</h2>
		{foreach from=$item key=name item=value}
			<div style="text-transform:lowercase;width:30%; float: left; ">{$name}:&nbsp;</div> 
			<div style="width:70%; float: right;background-color: #DADADA;">{$value.0.VALUE}&nbsp;</div>
			<div style='clear: both;'></div>
		{/foreach}
	</div>
	<br>
	{/foreach}
{/if}
