<h2>{t}Mail settings{/t}</h2>
<table style='width:100%;' cellspacing=0>
	<tr>
		<td style='width:50%;vertical-align:top; border-right:1px solid #AAA'>
			<table style="width:100%;">
				<tr>
					<td><label for="mail">{t}Primary address{/t}</label>{$must}</td>
                      {if !$isModifyableMail && $initially_was_account}
                       <input type='text' disabled size=30 value="{$mail}">
                      {else}
                       {if $domainSelectionEnabled}
                        <input type='text' id="mail" name="mail" size=20 maxlength=65 value="{$mail}">
                        @
                        <select name='MailDomain' size=1>
                         {html_options values=$MailDomains output=$MailDomains selected=$MailDomain}
                        </select>
                       {else}
                        <input type='text' id="mail" name="mail" size=35 maxlength=65 value="{$mail}">
                       {/if}
                      {/if}
                   </td>
				</tr>
				<tr>
					<td colspan="2">
						<p style="margin-bottom:0px;">
							<b><label for="forwarder_list">{t}Forward messages to{/t}</label></b>
						</p>
					</td>
				</tr>
			</table>
		</td>
		<td style="vertical-align:bottom;border-left:1px solid #A0A0A0">
			<table style="width:100%;">
				<tr>
				 	<td style="vertical-align:top;">
			   			<h2>
							<img class="center" alt="" align="middle" src="plugins/mail/images/alternatemail.png" />
							<label for="alternates_list">{t}Alternative addresses{/t}</label>
						</h2>
					</td>
				</tr>
			</table>
		</td>
	</tr>
	<tr>
		<td>
			<table style='width:100%;'>
				<tr>
					<td colspan="2">
					   	<select id="gosaMailForwardingAddress" style="width:100%; height:100px;" name="forwarder_list[]" size=15 multiple >
								{html_options values=$gosaMailForwardingAddress output=$gosaMailForwardingAddress}
								<option disabled>&nbsp;</option>
					   	</select>
						<br>
					    <input type='text' name="forward_address" size=20 align="middle" maxlength=65 	value="">
					    <input type="submit" value="{msgPool type=addButton}" name="add_forwarder"	>&nbsp;
					    <input type="submit" value="{t}Add local{/t}" name="add_local_forwarder"	>&nbsp;
					    <input type="submit" value="{msgPool type=delButton}" name="delete_forwarder"	>
					</td>
				</tr>
			</table>
		</td>
		<td style="vertical-align:bottom;border-left:1px solid #A0A0A0">
			<table style='width:100%;'>
				<tr>
					<td colspan="2" >
					   	<select id="alternates_list" style="width:100%;height:100px;" name="alternates_list[]" size="15"
							 multiple title="{t}List of alternative mail addresses{/t}"> 
								{html_options values=$gosaMailAlternateAddress output=$gosaMailAlternateAddress}
								<option disabled>&nbsp;</option>
					   	</select>
					   	<br/>
					   	<input type='text' name="alternate_address" size="30" align="middle" maxlength="65" value="">
			   			<input type=submit value="{msgPool type=addButton}" name="add_alternate">
			   			<input type=submit value="{msgPool type=delButton}" name="delete_alternate">
			  		</td>
				</tr>
			</table>
		</td>		
	</tr>
</table>
<br>
<br>
