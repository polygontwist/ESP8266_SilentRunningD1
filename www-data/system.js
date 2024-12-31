// V18.01.2024 Version 1


//RGB #060606 erstes glimmen
//

var typen={
	'ledWLAN':	{a:false,n:'LED Wlan',b:['LEDWLANON','LEDWLANOFF']}
};
var neoanimation=[];

var IOGroup,theMain,theneoNixels;

var onError=function(){
	if(IOGroup)IOGroup.error();
	if(theMain)theMain.error();
	addClass(document.getElementsByTagName('body')[0],"error");
}
var onOK=function(){
	subClass(document.getElementsByTagName('body')[0],"error");
}

var getpostData =function(url, auswertfunc,POSTdata,noheader,rh,getfiletype){
		var loader,i;
		try {loader=new XMLHttpRequest();}
		catch(e){
				try{loader=new ActiveXObject("Microsoft.XMLHTTP");}
				catch(e){
					try{loader=new ActiveXObject("Msxml2.XMLHTTP");}
					catch(e){loader=null;}
				}
			}	
		if(!loader)alert('XMLHttp nicht möglich.');
		var jdata=undefined;
		if(POSTdata!=undefined)jdata=POSTdata;//encodeURI
		
		loader.onreadystatechange=function(){
			if(loader.readyState==4){
				auswertfunc(loader);
				onOK();
				}
			};
		loader.ontimeout=function(e){console.log("TIMEOUT");}
		loader.onerror=function(e){console.log("ERR",e,loader.readyState);onError();}
		
		if(getfiletype==undefined)getfiletype='text/plain';
		
		if(jdata!=undefined){
				loader.open("POST",url,true);
				if(rh!=undefined){
						for(i=0;i<rh.length;i++){
							loader.setRequestHeader(rh[i].typ,rh[i].val);
						}
				}
				if(noheader!==true){
					loader.setRequestHeader("Content-Type","application/x-www-form-urlencoded");
					loader.setRequestHeader('Cache-Control','no-cache');
					loader.setRequestHeader("Pragma","no-cache");
					loader.setRequestHeader("Cache-Control","no-cache");
					jdata=encodeURI(POSTdata);
				}
				loader.send(jdata);
			}
			else{
				
				
				loader.open('GET',url,true);
				loader.setRequestHeader('Content-Type',getfiletype);
				loader.send(null);
			}
	}

var cE=function(ziel,e,id,cn){
	var newNode=document.createElement(e);
	if(id!=undefined)newNode.id=id;
	if(cn!=undefined)newNode.className=cn;
	if(ziel)ziel.appendChild(newNode);
	return newNode;
	}
var gE=function(id){return document.getElementById(id);}
var addClass=function(htmlNode,Classe){	
	var newClass;
	if(htmlNode!=undefined){
		newClass=htmlNode.className;
		if(newClass==undefined || newClass=="")newClass=Classe;
		else
		if(!istClass(htmlNode,Classe))newClass+=' '+Classe;	
		htmlNode.className=newClass;
	}			
}
var subClass=function(htmlNode,Classe){
	var aClass,i;
	if(htmlNode!=undefined && htmlNode.className!=undefined){
		aClass=htmlNode.className.split(" ");	
		var newClass="";
		for(i=0;i<aClass.length;i++){
			if(aClass[i]!=Classe){
				if(newClass!="")newClass+=" ";
				newClass+=aClass[i];
				}
		}
		htmlNode.className=newClass;
	}
}
var istClass=function(htmlNode,Classe){
	if(htmlNode.className){
		var i,aClass=htmlNode.className.split(' ');
		for(i=0;i<aClass.length;i++){
			if(aClass[i]==Classe)return true;
		}	
	}		
	return false;
}
var filterJSON=function(s){if(s=="")return{"error":"parseerror-empty"}
	var re=s;
	if(re.indexOf("'")>-1)re=re.split("'").join('"');
	try {re=JSON.parse(re);} 
	catch(e){
		console.log("JSON.parse ERROR:",s,":");
		re={"error":"parseerror"};
		}
	return re;
}
var cButt=function(z,txt,cl,data,click){
	var a=cE(z,"a");
	a.className=cl;
	a.innerHTML=txt;
	a.href="#";
	a.data=data;
	a.addEventListener('click', click);
	return a;
};
var onClickShowHide=function(e){
	if(istClass(this.data.z,"nodisplay")){
		subClass(this.data.z,"nodisplay")
		addClass(this,"buttison")
	}
	else{
		addClass(this.data.z,"nodisplay")
		subClass(this,"buttison")
	}
}

var rgbToHex=function(r, g, b){
	var componentToHex=function(c){
		var hex=c.toString(16);
		return hex.length==1?"0"+hex:hex;
	}
	return "#"+componentToHex(r)+componentToHex(g)+componentToHex(b);
}
var HexToRGB=function(s){
	s = s.slice(1);
	return [
		parseInt(s.slice(0, 2), 16),
		parseInt(s.slice(2, 4), 16),
		parseInt(s.slice(4, 6), 16)
		]
}


var chartobyte=function(s){
	var re=s.charCodeAt(0);
	if(re>255){
		console.log("konvertfehler",re,s);
		re=0;
	}
	return re;
}
var showbinhex=function(s){
	var i,charCode,re="";
	for(i=0;i<s.length;i++){
		console.log('#'+i,s.charCodeAt(i), ">"+s.charAt(i)+"<");
	}
}
	


var neoEditor=function(ziel){
	var isinit=false,data={},listnode
		,playerpos=0,timer,
		lastlistbutt;
	
	this.setdata=function(jdat){
		data=jdat;
		if(!isinit){
			isinit=true;
			ini();
		}
	}
	
	this.loadAni=function(dateiname){
		//getpostData("./"+dateiname,onLoadFile,undefined,false,undefined,'application/octet-stream');
		
		var node=gE("inputfilename");
		if(node)node.value=dateiname.split('.')[0];
		
		var xhr = new XMLHttpRequest();
		xhr.open('GET', "./"+dateiname, true);
		xhr.responseType = 'arraybuffer';

		xhr.onload = function() {
			if (xhr.status === 200) {
				loadAniinEdi(new Uint8Array(xhr.response));
			} else {
				console.error('Fehler beim Laden der Datei:', xhr.statusText);
			}
		};
		xhr.onerror = function() {
			console.error('Netzwerkfehler');
		};
		xhr.send();
	}
	
	var writeTabelhead=function(){
		//listnode
		var tr,th;
		tr=cE(listnode,"tr");
		th=cE(tr,"th");
		//th.innerHTML="Nr.";
		th=cE(tr,"th",undefined,"center");
		th.innerHTML="Anzeigedauer";
		th=cE(tr,"th",undefined,"center");
		th.innerHTML="Pixel";
		th=cE(tr,"th");
		th.innerHTML="Action";
		
	}
	
	var loadAniinEdi=function(filedata){//string
		var i=0,t,fbyte,zeit=0,pixelwerte=[],anz,r,g,b,bhi,hlow;
		
		neoanimation=[];
		listnode.innerHTML="";
		writeTabelhead();
		
		while(i<filedata.length){
			fbyte=filedata[i];
			if(fbyte==chartobyte("S")){//anzahl,r,g,b,r,g,b,...
				//chartobyte
				pixelwerte=[];
				i++;
				anz=filedata[i];
				for(t=0;t<anz;t++){
					i++;
					r=filedata[i];
					i++;
					g=filedata[i];
					i++;
					b=filedata[i];
					pixelwerte.push([r,g,b]);
				}
			}
			if(fbyte==chartobyte("T")){//2 byte=time
				//zeit=0
				i++;
				bhi=filedata[i];
				i++;
				hlow=filedata[i];
				zeit=bhi*256+hlow;
				
				addPixelline(pixelwerte,zeit);
			}
			i++
		}
	}
	
	var changeinp=function(e){
		this.dat.t=parseInt(this.value);
	}
	
	var onClickAdd=function(d){
		addPixelline(data.neoleds,1000)
	}
	
	
	var changepixelcolor=function(e){
		//console.log("**",this.value,this.data);
		var farbe=this.value;
		var label=this.data.label;
		label.style.backgroundColor=farbe;
		
		var pixel=HexToRGB(farbe);
		
		this.data.pixel[0]=pixel[0];
		this.data.pixel[1]=pixel[1];
		this.data.pixel[2]=pixel[2];
		
	}
	
	var addPixelline=function(pixeldata,time){
		var tr,td,i,p,px,q,inp,a,color,np,inp;
		var pixel=JSON.parse( JSON.stringify(pixeldata) );
		
		var ndat={pixel:[],t:time,nodes:[]};
		
		var nr=neoanimation.length;
		
		tr=cE(listnode,"tr");
		td=cE(tr,"td");
		td.innerHTML='#'+neoanimation.length;
		
		td=cE(tr,"td");
		inp=cE(td,"input");
		inp.type="number";
		inp.value=ndat.t;
		inp.min=10;
		inp.addEventListener("change",changeinp);
		inp.dat=ndat;
		inp=cE(td,"span");
		inp.innerHTML="ms";
		
		td=cE(tr,"td");
		for(i=0;i<pixel.length;i++){//[[r,g,b],[r,g,b],..]
			color='rgb('+pixel[i][0]+','+pixel[i][1]+','+pixel[i][2]+')';
			px=pixel[i];
						
			np=cE(td,"label","npZ"+nr+'_'+i,"npixel");
			np.setAttribute("for","inpZ"+nr+'_'+i);
			np.style.backgroundColor=color;
			
			inp=cE(td,"input","inpZ"+nr+'_'+i,"none");
			inp.setAttribute("type","color");
			inp.data={label:np, pixel:pixel[i]};
			inp.setAttribute("value",rgbToHex(pixel[i][0],pixel[i][1],pixel[i][2]));
			inp.addEventListener("change",changepixelcolor);
			
			/*
			p=cE(td,"span",undefined,"npixel");
			p.style.backgroundColor=color;
			*/
			ndat.pixel.push(pixel[i]);
			ndat.nodes.push(np);
			
			//inp.data["ndat"]=ndat;
		}
		
		td=cE(tr,"td");
		a=cButt(td,'von oben',"butt",{},onClickSettoLine);
		a.dat={d:ndat};
		
		
		a=cButt(td,"↑","butt",{},onClickMoveUp);
		a.dat={nr:neoanimation.length};
		if(neoanimation.length==0){
			a.removeEventListener('click',onClickMoveUp);
			addClass(a,"buttinaktiv");
		}
		
		if(lastlistbutt!=undefined){
			lastlistbutt.addEventListener('click',onClickMoveDown);
			subClass(lastlistbutt,"buttinaktiv");
		}
		
		a=cButt(td,"↓","butt buttinaktiv",{},function(){});
		a.dat={nr:neoanimation.length};
		lastlistbutt=a;
		
		a=cButt(td,'pixelset',"butt",{},onClickSettoPixel);
		a.dat={d:ndat};
		
		neoanimation.push(ndat);
	}
	
	var onClickSettoLine=function(e){
		var zdat=this.dat.d;
		var i,q,px,pixel=JSON.parse( JSON.stringify(data.neoleds) );
		for(i=0;i<zdat.nodes.length;i++){
			q=gE("np"+i);
			px=pixel[i];
			zdat.nodes[i].style.backgroundColor=q.style.backgroundColor;
			zdat.pixel[i]=q.color;
		}
		e.preventDefault();
	}
	
	var onClickSettoPixel=function(e){
		var zdat=this.dat.d,
			pixel=data.neoleds
			,i,zielnode,newval
			;
		for(i=0;i<zdat.nodes.length;i++){
			pixel[i]=zdat.pixel[i];
			zielnode=gE("np"+i);
			zielnode.style.backgroundColor=zdat.nodes[i].style.backgroundColor;
			zielnode=gE("inp"+i);
			newval=rgbToHex(pixel[i][0],pixel[i][1],pixel[i][2]);//#223344
			//console.log(pixel[i],newval);
			
			if(zielnode.value!=newval){
				zielnode.value=newval;
				//console.log(newval);
				getpostData("./action?led="+i+"&rgb="+newval.split('#').join('L'),function(){});
			}
		}
		e.preventDefault();
	}
	
	var onClickMoveUp=function(e){
		var qnr=this.dat.nr;
		tausche(qnr,qnr-1);
		e.preventDefault();
	}
	var onClickMoveDown=function(e){
		var qnr=this.dat.nr;
		tausche(qnr,qnr+1);
		e.preventDefault();
	}
	var tausche=function(n1,n2){
		var p1=neoanimation[n1];
		var p2=neoanimation[n2];
		var pxb=p2.pixel;
		p2.pixel=p1.pixel;
		p1.pixel=pxb;		
		setC(p1);
		setC(p2);
	}
	var setC=function(zeileninfo){
		var i;
		for(i=0;i<zeileninfo.nodes.length;i++){
			zeileninfo.nodes[i].style.backgroundColor="rgb("+zeileninfo.pixel[i].join(',')+')';
		}
	}
	
	var onClickNew=function(e){
		neoanimation=[];
		listnode.innerHTML="";
		writeTabelhead();
		
		var node=gE("inputfilename");
		if(node)node.value="";		
		e.preventDefault();
	}
	var onClickPlay=function(e){
		if(timer)clearTimeout(timer);
		playerpos=0;
		Ftimer();
		
		//todo: liste als Datei ablegen, play="datei.ani" ->binär wie .rgb 
		//rgbrgbrgbrgb
		//wait
		e.preventDefault();
	}
	
	var onClickSave=function(e){
		var i,f,name=this.input.value;
		
		if(neoanimation.length==0){
			alert("Keine Animation definert.");
			return;
		}
		if(name==""){
			alert("Bitte Name der Animation eingeben.");
			return;
		}
		
		//check files
		for(i=0;i<data.files.length;i++){
			f=data.files[i];
			if(f.fileName=='/'+name+'.ani'){
				if(!confirm("Datei überschreiben?")){
					return;
				}
			}
		}
		
		var datei=[],z,px,t,byte1,byte2,time;
		console.log(data);
		console.log(neoanimation);
		for(i=0;i<neoanimation.length;i++){
			z=neoanimation[i];
			pixel=z.pixel;
			datei.push(chartobyte("S"));
			datei.push(pixel.length);
			for(t=0;t<pixel.length;t++){
				px=pixel[t];
				datei.push(px[0]);
				datei.push(px[1]);
				datei.push(px[2]);
			}
			datei.push(chartobyte("T"));
			time=z.t;//ms
			byte1 = (time >> 8) & 0xFF; // Höheres Byte
            byte2 = time & 0xFF; 
			datei.push(byte1);//H
			datei.push(byte2);//L
		}
		datei.push(chartobyte("E"));
		
		var byteArray = new Uint8Array(datei);		
		var blob = new Blob([byteArray], { type: 'application/octet-stream' });
		var formData = new FormData();
		formData.append('file', blob, name+'.ani');
		var xhr = new XMLHttpRequest();
		xhr.open('POST', './upload?rel=no', true); // Ändern Sie die URL auf Ihre Upload-URL
		xhr.onload = function () {
			if (xhr.status === 200) {
				uploadauswertung(xhr);
			} else {
				uploadauswertung(xhr);
			}
		};
		xhr.send(formData);
		
		e.preventDefault();
	}
	
	
	
	var uploadauswertung=function(data){
		if(data.status==200){
			console.log("OK");
			location.reload();
		}
		else{
			console.log("uploadfehler ",data.status);
		} 
	}
	
	var Ftimer=function(){
		var i,v;
		if(playerpos<0)playerpos=0;
		if(playerpos>neoanimation.length-1)playerpos=0;
		
		var px=neoanimation[playerpos];
		if(px){
			for(i=0;i<px.pixel.length;i++){
				v=rgbToHex(px.pixel[i][0],px.pixel[i][1],px.pixel[i][2]);
				getpostData("./action?led="+i+"&rgb="+v.split('#').join('L'),function(d){});
			}			
			playerpos++;
			if(playerpos<neoanimation.length)
				timer=setTimeout(Ftimer,px.t);
		}
	}
	
	var ini=function(){
		var i,a,node,p,label;		
		p=cE(ziel,"p");
		a=cButt(p,'neue Liste',"butt",{},onClickNew);
		a=cButt(p,'add',"butt",{},onClickAdd);
		a=cButt(p,'play',"butt",{},onClickPlay);
		
		//input name
		node=cE(p,"input","inputfilename","inputfilename");
		node.setAttribute("maxlength",30-4);
		label=cE(p,"label");
		label.innerHTML=".ani";
		a=cButt(p,'save',"butt",{},onClickSave);
		a.input=node;
					
		listnode=cE(ziel,"table",undefined,"anitab");
	}
	
}



var neoNixels=function(){//Editor
	var mainnode
		,ziel
		,editornode,edy
		//,BRIGHTNESS=50//siehe auch arduinoscript
		
		;
	
	var fsetcolor=function(data){//wenn Befehl OK, Farbe von LED-Button setzen
		if(data.responseText.indexOf('OK')>-1){
			var i,a,lednr=-1,rgb=0,prop,
				jdat=filterJSON(data.responseText);
			for(i=0;i<jdat.Arguments.length;i++){
				a=jdat.Arguments[i];
				for(prop in a){
					if(prop=="led")lednr=parseInt(a[prop]);
					if(prop=="rgb")rgb=a[prop].split('L').join('#');
				}
			}
			if(lednr>-1){
				a=gE("np"+lednr);
				if(a){
					a.style.backgroundColor=rgb;//#189126
					if(rgb.indexOf('#')>-1)
						a.color=HexToRGB(rgb);
					}
			}
		}
	}
	var fset=function(data){}
	
	var changecolor=function(e){
		var v=this.value,i=this.getAttribute("data");
		if(v.indexOf("#")==0){
			getpostData("./action?led="+i+"&rgb="+v.split('#').join('L'),fsetcolor);
			//getpostData("./action?led="+i+"&rgb=-2288ff",fsetcolor);
		}
	}
	
	var changesavemode=function(e){
		var url="./action?set=saveneostatus"
		if(this.checked){
			getpostData(url+"ON",fset);
		}
		else{
			getpostData(url+"OFF",fset);
		}
	}
	

	this.setdata=function(jdat){
		//console.log(jdat.neoleds);
		var i,np,px,inp,node;
		if(jdat["saveneostatus"]){//true|false
			node=gE("saveneostatus");
			if(node==undefined){
				//create
				np=cE(ziel,"span");
				np.innerHTML="Status speichern:";
				
				var label;
				node=cE(ziel,"input","saveneostatus","booleanswitch");
				node.type="checkbox";
				node.addEventListener("change",changesavemode);
				label=cE(ziel,"label");
				label.htmlFor=node.id;
				addClass(label,"labelbooleanswitch");
				node.dataLabel=label;
				
				np=cE(ziel,"span");
				np.innerHTML="Neo:";

			}
			node.checked=jdat["saveneostatus"]==="true";
			//console.log(jdat["saveneostatus"]);
		}
		if(jdat["neoleds"]){
			for(i=0;i<jdat.neoleds.length;i++){
				px=jdat.neoleds[i];
				np=gE("np"+i);
				inp=gE("inp"+i);
				if(np==undefined){
					np=cE(ziel,"label","np"+i,"pout neopixel");
					np.setAttribute("for","inp"+i);
					inp=cE(ziel,"input","inp"+i,"none");
					inp.setAttribute("type","color");
					inp.setAttribute("data",i);
					inp.addEventListener("change",changecolor);
				}
				inp.setAttribute("value",rgbToHex(px[0],px[1],px[2]));
				np.style.backgroundColor="rgb("
					+px[0]+","
					+px[1]+","
					+px[2]+""					
					+")";
				np.color=px;
			}
			edy.setdata(jdat);
		}
	};
	
	this.loadAni=function(filename){
		edy.loadAni(filename);
	}
	
	var ini=function(){
		var z=document.getElementsByTagName('h1')[0];
		mainnode=document.createElement('div');
		z.insertAdjacentElement('afterend', mainnode);
		addClass(mainnode,"neopixelsmain");
		
		ziel=cE(mainnode,"div",undefined,"neopixels");
		
		editornode=cE(mainnode,"div",undefined,"neoedi");
		edy=new neoEditor(editornode);
	}
	ini();
}


var MainF=function(){
	var dateisysinfo="./data.json";
	var lokdat=undefined;	
	var wochentag=["Montag","Dienstag","Mittwoch","Donnerstag","Freitag","Samstag","Sonntag"];
	var sysinhalt;
	
	this.error=function(){}
	
	var changetimekorr=function(e){
		var val=this.value;//in Stunden
		//console.log(">>>",val);
		getpostData(dateisysinfo+'?settimekorr='+val,
			function(d){
				//console.log('reload data',d);
				setTimeout(function(){
					getpostData(dateisysinfo,fresultsysinfo);
					}
				,1000);//1 sec warten, bis intern Zeit gesetzt wurde
			}
		);
		//settimekorr, led=on, led=off
	}

	var anzinputs=0;
	var cI=function(ziel,typ,value,title){//create input
		var label;
		var input=cE(ziel,"input");
		input.type=typ;
		if(typ=="checkbox"){
			input.checked=value;
			input.id="cb"+anzinputs;
			label=cE(ziel,"label");
			label.htmlFor=input.id;
			input.dataLabel=label;
		}	
		else
			input.value=value;
		if(title!=undefined)input.title=title;	
		anzinputs++;
		return input;
	}

	
	var fresultsysinfo=function(data){//1x
		var ziel=gE('sysinfo'),
			jdat=filterJSON(data.responseText),
			div,node,p,a;
		
		if(theneoNixels)theneoNixels.setdata(jdat);
			
		if(ziel){//'sysinfo'
			ziel.innerHTML="";
			
			a=cButt(ziel,'System',"butt buttinfos",{z:sysinhalt},onClickShowHide);
			
			sysinhalt=cE(ziel,"div",undefined,"nodisplay");
			a.data.z=sysinhalt;
			
			div=cE(sysinhalt,"div",undefined,"utctimset");
			div.innerHTML="UTC Zeitunterschied:";
			var val=Math.floor(jdat.datum.timekorr);
			node=cI(div,"number",val,"Zeitunterschied");
			node.addEventListener('change',changetimekorr);
			node.maxlength=2;
			node.size=2;
			if(val==-1 || val==1)
				node=document.createTextNode(" Stunde");
			else
				node=document.createTextNode(" Stunden");
			div.appendChild(node);

			lokdat=cE(sysinhalt,"article");
			getlokaldata(jdat);
			
			node=document.getElementsByTagName('h1')[0];
			if(node)
				node.innerHTML=jdat.progversion+' '+jdat.hostname.split('Sonoff').join('');
			
			//fstotalBytes,fsusedBytes,fsused,progversion,aktionen
			//TODO:Filelist
			//portstatus .relais .led
			
		}
		checkplaingstatus(jdat);
		
	}
	var retlokaldata=function(data){
		jdat=filterJSON(data.responseText);
		getlokaldata(jdat);
	}
	var iftimr;
	var getlokaldata=function(jdat){//loop
		var node;
		if(iftimr!=undefined)clearTimeout(iftimr);
		
		if(jdat.bussy!=undefined){
			console.log("bussy:",jdat.bussy);
			iftimr=setTimeout(function(){
				getpostData(dateisysinfo,retlokaldata);
			},1000*5);//5sec
			return;
		}
		if(jdat.error!=undefined){
			console.log("Fehler",typeof jdat,jdat);
			iftimr=setTimeout(function(){
				getpostData(dateisysinfo,retlokaldata);
			},1000*20);//20sec
			return;
		}
		
		if(lokdat!=undefined){
			lokdat.innerHTML="";			
			node=cE(lokdat,"p");
			var t=jdat.lokalzeit.split(":");
			node.innerHTML="lokaltime: "+t[0]+':'+t[1];
			
			node=cE(lokdat,"p");
			s="";
			if(jdat.datum.day<10)s+="0";
			s+=jdat.datum.day+".";
			if(jdat.datum.month<10)s+="0";
			s+=jdat.datum.month+".";
			node.innerHTML="Datum: "+s+jdat.datum.year+" "+wochentag[jdat.datum.tag];
						
			node=cE(lokdat,"p");
			node.innerHTML="Sommerzeit: "+jdat.datum.summertime;
			
			node=cE(lokdat,"p");
			node.innerHTML="MAC: <span style=\"text-transform: uppercase;\">"+jdat.macadresse+"</span>";
			
			//console.log(jdat.files);
			
			if(IOGroup)IOGroup.refresh(jdat);
			
			iftimr=setTimeout(function(){
				getpostData(dateisysinfo,retlokaldata);
			},1000*5);//5sec
		
		
		
		}
		
		if(theneoNixels)theneoNixels.setdata(jdat);		
		
		checkplaingstatus(jdat);
	}
	var checkplaingstatus=function(jdat){
		var ziel,liste,i,t,tr,td,listetd;
		ziel=gE('flcontent');
		if(ziel){
			//anistatus setzen
			var anidateilink=jdat["anidateilink"];
			if(anidateilink!=undefined){
				anidateilink=anidateilink.split('/').join('');
				liste=ziel.getElementsByTagName("tr");
				for(i=0;i<liste.length;i++){
					tr=liste[i];
					listetd=tr.getElementsByTagName("td");
					if(listetd.length>0){
						td=listetd[0];
						if(td.data!=undefined){
							if(td.data["f"]==anidateilink && jdat["aniplaingstatus"]!="0"){
								//ist plaing
								addClass(td.data["bplay"],"hidden");
								subClass(td.data["bstop"],"hidden");
							}else{
								//ist nicht plaing
								subClass(td.data["bplay"],"hidden");
								addClass(td.data["bstop"],"hidden");
							}
						}
					}
				}
			}
		}
	}
	
	var holedat=function(){
		if(iftimr!=undefined)clearTimeout(iftimr);
		getpostData(dateisysinfo,retlokaldata);
	}
	
	var playmp3=function(e){
		console.log("play",this.data);
		getpostData("./action?mp3="+this.data,function(d){});
		e.preventDefault();
	}
	
	var playani=function(e){
		getpostData("./action?ani="+this.data,function(d){holedat()});
		e.preventDefault();
	}
	var stopani=function(e){
		getpostData("./action?stop=ani",function(d){holedat()});
		e.preventDefault();
	}
	
	var editani=function(e){
		if(theneoNixels)theneoNixels.loadAni(this.data);
		e.preventDefault();
	}
	
	var setdateioptionen=function(){
		var i,a,p,q,pnode,dateiname;
		var filebasis=gE("flcontent");
		q=filebasis.getElementsByTagName("a");
		
		for(i=0;i<q.length;i++){
			a=q[i];
			dateiname=a.innerHTML;
			
			if(a.innerHTML.indexOf('.mp3')>0){
				//addplay
				pnode=q[i].parentNode;//td
				pnode.data={"f":a.innerHTML};
				
				p=cE(pnode,"a",undefined,"buttplay");
				p.href="#";
				p.data=dateiname;
				p.innerHTML="play";
				p.addEventListener('click',playmp3);
				
				addClass(pnode.parentNode,"mp3file");
			}
			else
			if(a.innerHTML.indexOf('.ani')>0){
				//addplay
				pnode=q[i].parentNode;//td
				pnode.data={"f":dateiname};
				
				p=cE(pnode,"a",undefined,"buttplay");
				p.href="#";
				p.data=dateiname;
				p.innerHTML="play";
				p.addEventListener('click',playani);
				pnode.data["bplay"]=p;
				
				p=cE(pnode,"a",undefined,"buttstop hidden");
				p.href="#";
				p.data=dateiname;
				p.innerHTML="stop";
				p.addEventListener('click',stopani);
				pnode.data["bstop"]=p;
				
				p=cE(pnode,"a",undefined,"buttplay buttedit");
				p.href="#";
				p.data=dateiname;
				p.innerHTML="edit";
				p.addEventListener('click',editani);
				
				addClass(pnode.parentNode,"anifile");
			}
			else
			if(a.innerHTML.indexOf('.')>0){
				pnode=q[i].parentNode;//td
				addClass(pnode.parentNode,"file");
			}
		}
	}
	
	var ini=function(){
		var z2=gE('sysinfo');
		if(z2)getpostData(dateisysinfo,fresultsysinfo);//+'&time='+tim.getTime()
			
		setdateioptionen();
	}
	
	ini();
}



var oIOGroup=function(){//ausgeblendet
	var isinit=false,seturl="./action?set=",
		basisnode=gE("actions");
	
	this.refresh=function(data){refresh(data);}
	this.error=function(){}
	
	var refresh=function(data){
		if(!isinit){
			create(data);
		}		
		for(param in data.portstatus){ 
			o=typen[param];
			if(o && o.a){
				if(data.portstatus[param]){
					subClass(o.ostat,"inaktiv");
					subClass(o.obutt,"txtan");
					addClass(o.obutt,"txtaus");
					}
				else{
					addClass(o.ostat,"inaktiv");
					addClass(o.obutt,"txtan");
					subClass(o.obutt,"txtaus");
					}
			}
		}
	}
	
	var create=function(data){
		var param,z=basisnode,o,i,p,h2;
		if(basisnode){
			addClass(z,"nodisplay");
		}
		
		if(z && data.portstatus!=undefined){
			for(param in data.portstatus){ 
				if(typen[param])typen[param].a=true;//aktivieren	
			}
			//Interaktionen
			for(param in typen){
				o=typen[param];
				if(o.a){
					p=cE(z,"p");
					
					o.ostat=cE(p,"span",undefined,"pout c"+param+" inaktiv");					
					o.obutt =cButt(p,'',"butt",o.b,buttclick);
					
					h2=cE(p,"h2");
					h2.innerHTML=o.n;
				}
			}
					
			isinit=true;
		}
	}
		
	var buttclick=function(e){
		if(istClass(this,"txtan")){
			subClass(this,"txtan");
			addClass(this,"txtaus");
			getpostData(seturl+this.data[0],fresult);
		}
		else{
			addClass(this,"txtan");
			subClass(this,"txtaus");
			getpostData(seturl+this.data[1],fresult);
		}
		e.preventDefault();
	}	
	var fresult=function(data){
		var j=filterJSON(data.responseText);
		console.log(j);//befehl:"ok"
		//get status
		getpostData("./data.json",function(d){refresh(filterJSON(d.responseText))});
	}

}

var initFileList=function(){
	var ziel=gE("filelisttab"),cont=gE("flcontent");
	if(ziel!=undefined && cont!=undefined){
		a=cButt(ziel,'Dateiliste',"butt buttinfos",{z:cont},onClickShowHide);
		addClass(cont,"nodisplay");
	}
}


window.addEventListener('load', function (event) {
	theneoNixels=new neoNixels();
	IOGroup=new oIOGroup();
	theMain=new MainF();
	initFileList();
});
