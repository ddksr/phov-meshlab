<?php
$meta = empty($_GET) ? array() : $_GET;

$method = empty($meta['method']) ? 'get_id' : $meta['method'];
$id = empty($meta['id']) ? '' : $meta['id'];

function logger($vardump) {
	$fd = fopen("./log", "a");
	fwrite($fd, print_r($vardump, true));
	fclose($fd);
}

logger($meta);

function finish() {
	logger("finish");
	print "1";
}

function get_id() {
	logger($_POST);
	print '{"id": "54283d66c5675018be000000", "success": true}';
}

function download() {
	$file_url = './bunny.ply';
	header('Content-Type: application/octet-stream');
	header("Content-Transfer-Encoding: Binary"); 
	header("Content-disposition: attachment; filename=\"" . basename($file_url) . "\""); 
	readfile($file_url);
}

function check_download() {
	print '' . round(rand(0,1));
}

function upload() {
	logger($_FILES);
	logger(getallheaders());
	logger(array(
		"body" => http_get_request_body()
	));
	print "1";
}
logger('------------');
logger($_GET);
switch($method) {
	case "download":
		download();
		break;
	case "check_download":
		check_download();
		break;
	case "upload":
		upload();
		break;
	case "get_id":
	default:
		get_id();
		
}
