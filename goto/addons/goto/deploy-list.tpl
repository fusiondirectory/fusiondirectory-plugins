<input type='image' src='images/empty.png' name='no_action_posted' value='do_nothing' alt='' style='width:2px;height:2px;'>

<table summary="" style="width:100%;height:100%; vertical-align:top; text-align:left;" cellpadding=2>
  <tr>
    <td style="vertical-align:top; width:100%;">
      <div class="contentboxh">
        <p class="contentboxh">&nbsp;{$HEADLINE}&nbsp;{$SIZELIMIT}</p>
      </div>
      
      <div class="contentboxb">
       <div style='background:white;padding:3px;'>
        <table><tr>
          <td>{$ROOT}&nbsp;</td><td>{$BACK}&nbsp;</td><td>{$HOME}&nbsp;</td><td>{$RELOAD}&nbsp;</td><td>{$SEPARATOR}&nbsp;</td><td>{t}Base{/t} {$BASE}&nbsp;</td><td>{$SEPARATOR}&nbsp;</td><td><img src='images/rocket.png' alt='' class='center'></td><td> {$ACTIONS}</td>
        </tr></table>
       </div>
      </div>
      
      <div style='height:4px;'>
      </div>
      {$LIST}
    </td>
  </tr>
</table>

<input type="hidden" name="ignore">


<!--
JS to reload the progress bars.

-->
{literal}
<script type="text/javascript">

/* Get request object handler for this type of browser 
 */
if (typeof XMLHttpRequest != 'undefined')
{
    xmlHttpObject = new XMLHttpRequest();
}
if (!xmlHttpObject)
{
    try
    {
        xmlHttpObject = new ActiveXObject("Msxml2.XMLHTTP");
    }
    catch(e)
    {
        try
        {
            xmlHttpObject = new ActiveXObject("Microsoft.XMLHTTP");
        }
        catch(e)
        {
            xmlHttpObject = null;
        }
    }
}

var fai_status = new Array();

function loadContent()
{
	var c = 0;

	/* Create array of available progress images once 
	 */
	if(!fai_status.length){
		for (var i = 0; i < document.images.length; i++) {
			var img = document.images[i];
			var id  = img.id;
			if(id.match(/^progress_/)){
				var mac = id.replace(/^progress_/,''); 
				mac = mac.replace(/_/g,':'); 
				fai_status[c] = new Object();
				fai_status[c]['MAC']  = mac;
				fai_status[c]['PROGRESS'] = -1;
				c ++;
			}
		}
	}

	/* Create string of macs used as parameter for getFAIstatus.php
		to retrieve all progress values.
     */
	var macs = "";
	for (var i = 0; i < fai_status.length; i++) {
		macs += fai_status[i]['MAC'] + ","
	}

	/* Send request 
     */
    xmlHttpObject.open('get','getFAIstatus.php?mac=' + macs);
    xmlHttpObject.onreadystatechange = handleContent;
    xmlHttpObject.send(null);
    return false;
}


function handleContent()
{
    if (xmlHttpObject.readyState == 4)
    {
		/* Get text and split by newline 
         */
        var text = xmlHttpObject.responseText;
		var data = text.split("\n");

		/* Walk through progress images and check if the 
		   progress status has changed 
		 */
		for (var e = 0; e < fai_status.length; e++) {
		
			/* Walk through returned values and parse out 
			   mac and progress value */
			var found 	= false;

			/* Create object id out of mac address 
               12:34:56:12:34:56 => progress_12_34_56_12_34_56
             */
			var id 		= fai_status[e]["MAC"].replace(/:/g,"_"); 
			id = "progress_" + id;
			var img = document.getElementById(id);	

			/* Continue if there is no image object iwth this id 
		     */
			if(!img){
			 	continue;
			}

			for (var i = 0; i < data.length; i++) {
				var mac 	= data[i].replace(/\|.*$/,"");
				var progress= data[i].replace(/^.*\|/,"");

				/* Match mac returned by the support daemon and 
					the one out of our list */
				if(fai_status[e]["MAC"] == mac){
					found = true;	

					/* Check if progress has changed 
					 */	
					if(fai_status[e]["PROGRESS"] != progress){
						img.src = "progress.php?x=80&y=13&p=" + progress; 
						fai_status[e]["PROGRESS"] = progress;
					}
					break;
				}
			}
			//document.getElementById("text1").value += "\n ";

			/* There was no status send for the current mac. 
			   This means it was removed from the queue.
			 */
			if(!found){
				document.mainform.submit();				
			}
		}
		timer=setTimeout('loadContent()',3000);
    }
}

timer=setTimeout('loadContent()',3000);
</script>
{/literal}
