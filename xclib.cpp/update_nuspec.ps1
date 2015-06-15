param([string]$project)

$date_now = get-date
$date_build_start = get-date -year $date_now.year -month $date_now.month -day $date_now.day -hour 0 -minute 0 -second 0
$date_rev_start = get-date -year 2000 -month 1 -day 1 -hour 0 -minute 0 -second 0

$build = [int][math]::Ceiling((New-TimeSpan -start $date_rev_start -end $date_now).TotalDays)
$revision = [int][math]::Floor((New-TimeSpan -start $date_build_start -end $date_now).TotalSeconds / 2)

$path = $project + ".nuspec"
$xml = [xml](get-content $path)
$version = $xml.package.metadata.version.Split(".")
$version[2] = $build
$version[3] = $revision
$xml.package.metadata.version = [string]::Join(".", $version)
$xml.package.metadata.id = $project
$xml.package.metadata.title = $project
$xml.Save($path)