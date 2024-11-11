<body onLoad="javascript:$$('div.debug_div').each(function (a) { a.hide(); });">

{$php_errors}
{$msg_dialogs}

<html lang="EN">
<div id="window-container">

    <div id="window-div">
        <!-- Simplified form that submits taskName, token, and user ID -->
        <form action='accountProlongation.php' method='post' name='mainform'>
            <input type="hidden" name="CSRFtoken" value="{$CSRFtoken}"/>
            <div id="window-titlebar">
                <p>
                    <img class="center" src="geticon.php?context=types&amp;icon=user&amp;size=48" alt="{t}User{/t}" title="{t}User{/t}"/>
                    {t}Reminder - Account prolongation{/t}
                </p>
            </div>
           {if !$success && !$error}
               <div style="text-align: center;">
                   <label for="login">
                       <img class="center" src="geticon.php?context=types&amp;icon=user&amp;size=48" alt="{t}Username{/t}" title="{t}Username{/t}"/>&nbsp;
                   </label>
                   <input type="text" name="login" placeholder="{t}Enter your ID{/t}" required />
                   <br />

                   <!-- Dropdown for selecting the type of ID -->
                   <label for="auth">{t}Select ID Type{/t}:</label>
                   <select name="auth" id="auth" required>
                       <option value="mail">{t}Mail{/t}</option>
                       <option value="uid">{t}UID{/t}</option>
                       <option value="supannAlias">{t}Supann Alias{/t}</option>
                       <option value="EPPN">{t}EPPN{/t}</option>
                   </select>
                   <input type="submit" name="apply" value="{t}Submit{/t}" title="{t}Click here to prolonged your account.{/t}"/>

                   <!-- Hidden fields for taskName and token from $_GET -->
                   <input type="hidden" name="taskName" value="{$taskName}" />
                   <input type="hidden" name="token" value="{$token}" />
               </div>

               <div id="window-footer" class="plugbottom">
               </div>
           {/if}
        </form>

        <div id="window-content">
            <div>
                <!-- Display confirmation message if the form was successfully submitted -->
                {if $success}
                    <div class="success">
                        <img class="center" src="geticon.php?context=status&amp;icon=task-complete&amp;size=16" alt="{t}Success{/t}" title="{t}Success{/t}">&nbsp;
                        <b>{t}Your account has been prolonged successfully.{/t}</b><br/>
                    </div>
                {else if $error}
                    <div class="error">
                        <img class="center" src="geticon.php?context=status&amp;icon=task-failure&amp;size=16" alt="{t}Error{/t}" title="{t}Error{/t}">&nbsp;
                        <b>{t}Error, data received are invalid!{/t}</b>
                    </div>
                {/if}
                <br/>
                <a href="./">Return to login screen</a>
            </div>
        </div>
    </div>
</div>
</body>
<script type="text/javascript">
    <!-- // Error Popup
    next_msg_dialog();
    -->
</script>
</html>
