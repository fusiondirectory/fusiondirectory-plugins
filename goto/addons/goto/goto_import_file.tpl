<h2>{t}Import jobs{/t}</h2>
<p>
{t}You can import a list of jobs into the GOsa job queue. This should be a semicolon seperated list of items in the following format:{/t}
</p>
<i>{t}timestamp{/t} ; {t}MAC-address{/t} ; {t}job type{/t} ; {t}object group{/t} [ ; {t}import base{/t} ; {t}full hostname{/t} ; {t}IP-address{/t} ; {t}DHCP group{/t} ]</i>
<br>
<br>
{if !$count}
{t}Example{/t}:
<br>
20080626162556 <b>;</b> 00:0C:29:99:1E:37 <b>;</b> job_trigger_activate_new <b>;</b> goto-client <b>;</b> dc=test,dc=gonicus,dc=de
<br>
<br>
{/if}

<p class="seperator"></p>
&nbsp;
<table>
	<tr>	
		<td>
			{t}Select list to import{/t}
		</td>
		<td>
			<input type='file' name='file' value="{t}Browse{/t}">
			<input type='submit' name='import' value='{t}Upload{/t}'>
		</td>
	</tr>
</table>

	{if  $count}
		<p class="seperator">&nbsp;</p>
		<br>
		<br>
		<div style='width:100%; height:300px; overflow: scroll;'>
		<table cellpadding="3" cellspacing="0" style='width:100%; background-color: #CCCCCC; border: solid 1px #CCCCCC;'>
			<tr>
				<td><b>{t}Timestamp{/t}</b></td>
				<td><b>{t}MAC{/t}</b></td>
				<td><b>{t}Event{/t}</b></td>
				<td><b>{t}Object group{/t}</b></td>
				<td><b>{t}Base{/t}</b></td>
				<td><b>{t}FQDN{/t}</b></td>
				<td><b>{t}IP{/t}</b></td>
				<td><b>{t}DHCP{/t}</b></td>
			</tr>
		{foreach from=$info item=item key=key}
			{if $item.ERROR}
				<tr style='background-color: #F0BBBB;'>
					<td>{$item.TIMESTAMP}</td>
					<td>{$item.MAC}</td>
					<td>{$item.HEADER}</td>
					<td>{$item.OGROUP}</td>
					<td>{$item.BASE}</td>
					<td>{$item.FQDN}</td>
					<td>{$item.IP}</td>
					<td>{$item.DHCP}</td>
				</tr>	
				<tr style='background-color: #F0BBBB;'>
					<td colspan="7"><b>{$item.ERROR}</b></td>
				</tr>
			{else}
				{if ($key % 2)}
					<tr class="rowxp0"> 
				{else}
					<tr class="rowxp1"> 
				{/if}
					<td>{$item.TIMESTAMP}</td>
					<td style='border-left: solid 1px #BBBBBB;'>{$item.MAC}</td>
					<td style='border-left: solid 1px #BBBBBB;'>{$item.HEADER}</td>
					<td style='border-left: solid 1px #BBBBBB;'>{$item.OGROUP}</td>
					<td style='border-left: solid 1px #BBBBBB;'>{$item.BASE}</td>
					<td style='border-left: solid 1px #BBBBBB;'>{$item.FQDN}</td>
					<td style='border-left: solid 1px #BBBBBB;'>{$item.IP}</td>
					<td style='border-left: solid 1px #BBBBBB;'>{$item.DHCP}</td>
				</tr>
			{/if}
		{/foreach}
		</table>
		</div>
	{/if}
<br>
<p class='seperator'></p>
<div style='text-align:right;width:99%; padding-right:5px; padding-top:5px;'>
	<input type='submit' name='start_import' value='{t}Import{/t}' >&nbsp;
	<input type='submit' name='import_abort' value='{msgPool type=backButton}'>
</div>
<br>
